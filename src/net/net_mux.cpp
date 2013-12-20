// vim: set noet:

/** \file */

#include "net_error.h"
#include "net_mux.h"
#include <algorithm>
#include <atomic>
#include <cstring>
#include <sstream>
#include <stdexcept>
#include <sys/epoll.h>
#include <sys/eventfd.h>

namespace clane {
	namespace net {

		mmux::sig_entry::sig_entry(std::shared_ptr<signal> const &sig): sig(sig), stat(inactive), read_readiness(false),
	   	write_readiness(false), timeout_readiness(false), detach_readiness(false), gc_cnt(0) {
		}

		mmux::wait_context::~wait_context() {
			if (waiting) {
					std::lock_guard<std::mutex> gc_lock(owner->gc_mutex);
					auto p = std::find(owner->wait_ctx_list.begin(), owner->wait_ctx_list.end(), this);
					owner->wait_ctx_list.erase(p);
			}
		}

		mmux::wait_context::wait_context(mmux *owner): owner(owner), waiting{},
		 	gc_queue_pos(owner->gc_queue.end()) {
		}

		mmux::~mmux() noexcept {
			terminate();
		}

		mmux::mmux(): thrd_cnt{}, term_start{}, timeout_waiting(false) {

			// create epoll object:
			file_descriptor local_epoll_fd = epoll_create1(0);
			if (-1 == local_epoll_fd) {
				std::ostringstream ss;
				ss << "error creating epoll object: " << errno_to_string(errno);
				throw std::runtime_error(ss.str());
			}

			// create control event object, set it to a signaled state, and add it to
			// the epoll object disabled:
			file_descriptor local_ctrl_fd = eventfd(0, 0);
			if (-1 == local_ctrl_fd) {
				std::ostringstream ss;
				ss << "error creating event object: " << errno_to_string(errno);
				throw std::runtime_error(ss.str());
			}
			int status = eventfd_write(local_ctrl_fd, 1);
			if (-1 == status) {
				std::ostringstream ss;
				ss << "error writing to event object: " << errno_to_string(errno);
				throw std::runtime_error(ss.str());
			}
			epoll_event ev{};
			status = epoll_ctl(local_epoll_fd, EPOLL_CTL_ADD, local_ctrl_fd, &ev);
			if (-1 == status) {
				std::ostringstream ss;
				ss << "error adding event object to epoll object: " << errno_to_string(errno);
				throw std::runtime_error(ss.str());
			}

			// done:
			swap(epoll_fd, local_epoll_fd);
			swap(ctrl_fd, local_ctrl_fd);
		}

		void mmux::attach_signal(std::shared_ptr<signal> const &sig) {

			sig_entry *ent;
			{
				std::lock_guard<std::mutex> term_lock(term_mutex);
				if (term_start)
					return;

				// insert new entry into the Signal Map:
				std::unique_ptr<sig_entry> se(new sig_entry(sig));
				ent = se.get();
				std::lock_guard<std::mutex> sig_map_lock(sig_map_mutex);
				sig_map[ent] = std::move(se);
				sig->owner = this;
				sig->ent = ent;
			}

			// RAII: detach signal on error:
			struct sig_map_guard {
				mmux *mux;
				sig_entry *ent;
				~sig_map_guard() { if(ent) { mux->detach_signal(ent); }}
			} detacher{this, ent};

			// update timeout state:
			bool earlier = false;
			if (sig->is_timeout_set()) {
				std::lock_guard<std::mutex> timeout_lock(timeout_mutex);
				earlier = timeout_queue_empty() || sig->timeout < timeout_queue_front()->sig->timeout;
				timeout_queue_insert(ent);
			}
			if (earlier)
				wake_one();

			// Now that the timeout is updated, the signal may be accessed by a
			// multiplexer thread at any time.

			// add to the epoll object:
			int flags = sig->initial_event_flags();
			epoll_event ev{};
			ev.events = EPOLLET;
			ev.events |= (flags & signal::read_flag) ? EPOLLIN : 0;
			ev.events |= (flags & signal::write_flag) ? EPOLLOUT : 0;
			ev.data.ptr = ent;
			int status = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, sig->fd(), &ev);
			if (-1 == status) {
				std::ostringstream ss;
				ss << "error adding file descriptor to epoll object: " << errno_to_string(errno);
				throw std::runtime_error(ss.str());
			}

			// success:
			detacher.ent = nullptr;
		}

		void mmux::detach_signal(sig_entry *ent) {
			{
				std::lock_guard<std::mutex> ready_lock(ready_mutex);
				ent->detach_readiness = true;
				if (ent->is_waiting())
					ready_queue_push(ent);
			}
			wake_one();
		}

		void mmux::detach_signal(signal *sig) {
			detach_signal(reinterpret_cast<sig_entry *>(sig->ent));
		}

		void mmux::detach_signal_entry(sig_entry *ent) {

			signal * const sig = ent->sig.get();
			sig->detaching();

			// remove file descriptor from epoll object:
			int status = epoll_ctl(epoll_fd, EPOLL_CTL_DEL, sig->fd(), nullptr);
			if (-1 == status) {
				std::ostringstream ss;
				ss << "error removing file descriptor from epoll object: " << errno_to_string(errno);
				throw std::runtime_error(ss.str());
			}

			// release now or begin garbage collection:
			//
			// Garbage collection: Though the signal's file descriptor is
			// removed from the epoll object, any other multiplexer thread
			// that's in the Wait State may try to access the signal. Because
			// this thread can't afford to wait for any other thread, the signal
			// is added to the Garbage Collection Queue, so that eventually
			// another thread will release the multiplexer's reference to the
			// signal.
			{
				std::lock_guard<std::mutex> gc_lock(gc_mutex);
				if (wait_ctx_list.empty()) {
					release_signal(ent);
				} else {
					// add to Garbage Collection Queue:
					auto pos = gc_queue.insert(gc_queue.end(), ent);
					for (auto p = wait_ctx_list.begin(); p != wait_ctx_list.end(); ++p) {
						++ent->gc_cnt;
						if ((*p)->gc_queue_pos == gc_queue.end())
							(*p)->gc_queue_pos = pos;
					}
				}
			}

		}

		void mmux::run() {

			// Check that the multiplexer isn't terminating or terminated.
			{
				std::lock_guard<std::mutex> term_lock(term_mutex);
				if (term_start)
					return;
				++thrd_cnt;
			}

			struct thread_decrement {
				mmux *mux;
				thread_decrement(mmux *mux): mux(mux) {}
				~thread_decrement() {
					std::lock_guard<std::mutex> term_lock(mux->term_mutex);
					--mux->thrd_cnt;
					mux->term_cond.notify_all();
				}
			} thrd_cnt_dec{this};

			// Each multiplexer thread has a unique context.
			wait_context local_ctx(this);

			int to = -1; // epoll timeout
			bool term = false; // current thread's awareness of whether multiplexer is terminating

			while (true) {

				// enter Wait state:
				{
					std::lock_guard<std::mutex> gc_lock(gc_mutex);
					wait_ctx_list.push_back(&local_ctx);
					local_ctx.waiting = true;
				}

				// determine epoll timeout period:
				// At most one multiplexer thread will check for a signal timeout, and
				// that thread will check for a timeout of the signal with the soonest
				// timeout time. 
				bool local_timeout_waiting = false;
				if (!term && -1 == to) {
					std::unique_lock<std::mutex> timeout_lock(timeout_mutex);
					if (!timeout_queue_empty() && !timeout_waiting) {
						timeout_waiting = true;
						sig_entry *ent = timeout_queue_front();
						auto timeout = ent->sig->timeout;
						timeout_lock.unlock();
						local_timeout_waiting = true;
						auto now = std::chrono::steady_clock::now();
						if (now >= timeout) {
							to = 0; // timeout has expired--don't block on epoll
						} else {
							to = (timeout - now) / std::chrono::milliseconds(1);
						}
					}
				}

				// wait for an I/O event:
				static int constexpr event_cap = 1;
				epoll_event ev[event_cap];
				int num_events = 0;
				if (!term) {
					do {
						num_events = epoll_wait(epoll_fd, ev, event_cap, to);
					} while (-1 == num_events && EINTR == errno);
					if (-1 == num_events) {
						std::ostringstream ss;
						ss << "error waiting on epoll object: " << errno_to_string(errno);
						throw std::runtime_error(ss.str());
					}
				}

				// Interpret the I/O events.

				for (int i = 0; i < num_events; ++i) {

					// event object readiness:
					if (!ev[i].data.ptr) {
						std::lock_guard<std::mutex> term_lock(term_mutex);
						if (term_start) {
							term = true;
							wake_one();
						}
					}

					// signal readiness:
					else {
						sig_entry *ent = static_cast<sig_entry *>(ev[i].data.ptr);
						std::lock_guard<std::mutex> ready_lock(ready_mutex);
						if (ent->is_waiting())
							ready_queue_push(ent);
						if (ev[i].events & EPOLLIN)
							ent->read_readiness = true;
						if (ev[i].events & EPOLLOUT)
							ent->write_readiness = true;
						// FIXME: What to do with EPOLLERR and EPOLLHUP?
					}
				}

				// check for signal timeouts:
				{
					auto now = std::chrono::steady_clock::now();
					std::unique_lock<std::mutex> timeout_lock(timeout_mutex);
					if (local_timeout_waiting)
						timeout_waiting = false;
					sig_entry *ent;
					signal *sig;
					while (!timeout_queue_empty() && now >= (sig = (ent = timeout_queue_front())->sig.get())->timeout) {
						timeout_queue_pop_front();
						sig->reset_timeout();
						timeout_lock.unlock();
						{
							std::lock_guard<std::mutex> ready_lock(ready_mutex);
							ent->timeout_readiness = true;
							if (ent->is_waiting())
								ready_queue_push(ent);
						}
						timeout_lock.lock();
					}
				}

				{
					std::unique_lock<std::mutex> gc_lock(gc_mutex);

					// garbage collection for this thread:
					while (local_ctx.gc_queue_pos != gc_queue.end()) {
						sig_entry *ent = *local_ctx.gc_queue_pos;
						--ent->gc_cnt;
						if (!ent->gc_cnt) {
							gc_lock.unlock(); // unlock is unnecessary--but may help performance?
							release_signal(ent);
							gc_lock.lock();
							++local_ctx.gc_queue_pos;
							gc_queue.pop_front();
						} else {
							++local_ctx.gc_queue_pos;
						}
					}

					// exit Wait state:
					auto p = std::find(wait_ctx_list.begin(), wait_ctx_list.end(), &local_ctx);
					wait_ctx_list.erase(p);
					local_ctx.waiting = false;
				}

				// Handle some ready signals.

				static int constexpr max_handle_cnt = 32;
				int handle_cnt = 0;
				std::unique_lock<std::mutex> ready_lock(ready_mutex);
				while (handle_cnt < max_handle_cnt && !ready_queue_empty()) {

					// pop signal from queue:
					sig_entry * const ent = ready_queue_pop();
					signal * const sig = ent->sig.get();
					++handle_cnt;
					ent->set_as_in_progress();

					// read:
					if (ent->read_readiness && !ent->detach_readiness) {
						ent->read_readiness = false;
						ready_lock.unlock();
						signal::ready_result opstat = sig->read_ready();
						ready_lock.lock();
						if (signal::ready_result::op_incomplete == opstat) {
							ent->read_readiness = true;
						} else if (signal::ready_result::signal_complete == opstat) {
							ent->detach_readiness = true;
						}
					}

					// write:
					if (ent->write_readiness && !ent->detach_readiness) {
						ent->write_readiness = false;
						ready_lock.unlock();
						signal::ready_result opstat = sig->write_ready();
						ready_lock.lock();
						if (signal::ready_result::op_incomplete == opstat) {
							ent->write_readiness = true;
						} else if (signal::ready_result::signal_complete == opstat) {
							ent->detach_readiness = true;
						}
					}

					// timeout:
					if (ent->timeout_readiness && !ent->detach_readiness) {
						ent->timeout_readiness = false;
						ready_lock.unlock();
						sig->timed_out();
						ready_lock.lock();
					}

					// detach:
					if (ent->detach_readiness) {
						ready_lock.unlock();
						detach_signal_entry(ent);
						ready_lock.lock();
						continue;
					}

					// INVARIANT: The current thread holds the Ready Mutex.

					// push signal into Ready Queue if signal is still ready:
					if (ent->read_readiness || ent->write_readiness || ent->timeout_readiness) {
						ready_queue_push(ent);
					}

					// otherwise signal is kept out of the Ready Queue and becomes active
					// again only after an epoll wakeup:
					else {
						ent->set_as_waiting();
					}
				}

				if (term && ready_queue_empty())
					return;
				if (!ready_queue_empty())
					to = 0; // don't block on next epoll_wait
				else
					to = -1; // block on next epoll_wait indefinitely
			}
		}

		void mmux::terminate() {
			std::unique_lock<std::mutex> ready_lock(ready_mutex);
			std::unique_lock<std::mutex> term_lock(term_mutex);
			if (term_start)
				return;
			term_start = true;
			// The multiplexer is now terminating. No new signals will be attached. No
			// new multiplexer threads will run. Mark all currently attached signals
			// as Detach Ready.
			for (auto i = sig_map.begin(); i != sig_map.end(); ++i) {
				sig_entry *ent = i->first;
				ent->detach_readiness = true;
				if (ent->is_waiting())
					ready_queue_push(ent);
			}
			if (!thrd_cnt) {
				// The multiplexer is imminently terminated. Nevertheless, signals may
				// still be attached. This may happen if multiplexer threads exited
				// because of a nonrecoverable error or if no multiplexer threads ever
				// ran. Detach any signals that are still attached.
				while (!ready_queue_empty()) {
					sig_entry *ent = ready_queue_pop();
					term_lock.unlock();
					ready_lock.unlock();
					detach_signal_entry(ent);
					ready_lock.lock();
					term_lock.lock();
				}
				// Now the multiplexer is terminated.
				term_cond.notify_all();
			} else {
				wake_one(); // multiplexer must shut down
			}
		}

		void mmux::wait() {
			std::unique_lock<std::mutex> term_lock(term_mutex);
			while (!term_start || thrd_cnt) {
				term_cond.wait(term_lock);
			}
		}

		void mmux::release_signal(sig_entry *ent) {
			{
				std::lock_guard<std::mutex> timeout_lock(timeout_mutex);
				if (ent->sig->is_timeout_set())
					timeout_queue_erase(ent);
			}
			std::shared_ptr<signal> sig;
			{
				std::lock_guard<std::mutex> lock(sig_map_mutex);
				ent->sig->owner = nullptr;
				sig = std::move(ent->sig); // signal in map is now invalid
				sig_map.erase(ent);
			}
			sig->detached();
		}

		void mmux::set_signal_timer(signal *sig, std::chrono::steady_clock::time_point const &new_timeout) {
			sig_entry *ent = reinterpret_cast<sig_entry *>(sig->ent);
			bool earlier;
			{
				std::lock_guard<std::mutex> timeout_lock(timeout_mutex);
				// Remove, add or reorder the signal in the Timeout Queue.
				if (sig->is_timeout_set())
					timeout_queue_erase(ent);
				sig->timeout = new_timeout;
				earlier = timeout_queue_empty() || new_timeout < timeout_queue_front()->sig->timeout;
				if (sig->is_timeout_set())
					timeout_queue_insert(ent);
			}
			// If the new timeout precedes all existing timeouts then wake up a
			// multiplexer thread so that it can reset its epoll timeout value.
			if (earlier)
				wake_one();
		}

		void mmux::wake_one() {
			// Enable the event object to wake up one multiplexer thread.
			epoll_event ev{};
			ev.events = EPOLLIN | EPOLLONESHOT;
			ev.data.ptr = nullptr;
			int status = epoll_ctl(epoll_fd, EPOLL_CTL_MOD, ctrl_fd, &ev);
			if (-1 == status) {
				std::ostringstream ss;
				ss << "error adding event object to epoll object: " << errno_to_string(errno);
				throw std::runtime_error(ss.str());
			}
		}

		bool mmux::ready_queue_empty() const noexcept {
			return ready_queue.empty();
		}

		mmux::sig_entry *mmux::ready_queue_pop() noexcept {
			sig_entry *ent = ready_queue.front();
			ready_queue.pop_front();
			return ent;
		}

		void mmux::ready_queue_push(sig_entry *ent) noexcept {
			ready_queue.push_back(ent);
			ent->set_as_queued();
		}

		bool mmux::timeout_queue_empty() const noexcept
		{
			return timeout_queue.empty();
		}

		void mmux::timeout_queue_pop_front() noexcept
		{
			timeout_queue.erase(timeout_queue.begin());
		}

		mmux::sig_entry *mmux::timeout_queue_front() const noexcept
		{
			return timeout_queue.begin()->second;
		}

		void mmux::timeout_queue_erase(sig_entry *ent) noexcept
		{
			// It's possible though unlikely that multiple signals have the same
			// timeout value. Remove only the given signal from the Timeout Queue.
			auto range = timeout_queue.equal_range(ent->sig->timeout);
			auto pos = std::find_if(range.first, range.second,
					[&](std::multimap<std::chrono::steady_clock::time_point, sig_entry *>::value_type const &key) {
				 	return ent == key.second; });
			timeout_queue.erase(pos);
		}

		void mmux::timeout_queue_insert(sig_entry *ent) noexcept
		{
			timeout_queue.insert(std::multimap<std::chrono::steady_clock::time_point, sig_entry *>::value_type(ent->sig->timeout, ent));
		}

		signal::signal(): owner(nullptr) {
		}

		void signal::clear_timer() {
			set_timer(std::chrono::steady_clock::time_point());
		}

		void signal::set_timer(std::chrono::steady_clock::duration const &timeout) {
			set_timer(std::chrono::steady_clock::now() + timeout);
		}

		void signal::set_timer(std::chrono::steady_clock::time_point const &timeout) {
			if (owner)
				owner->set_signal_timer(this, timeout);
			else
				this->timeout = timeout;
		}

		void signal::timed_out() {
			// Default behavior is to do nothing.
		}

		void signal::detach() {
			owner->detach_signal(this);
		}

		void signal::detaching() {}

		void signal::detached() {}

	}
}

