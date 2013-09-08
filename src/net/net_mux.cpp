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

		// Mutex acquisition order
		//
		// To prevent deadlocks, mutexes must be acquired in a consist order when
		// two or more mutexes are acquired simultaneously.
		//
		// Signal before Ready Queue
		// Signal before Wait Context before Garbage Collection Queue
		// Signal before Timeout Queue
		//

		// Represents the existence of a multiplexer thread.
		class mux_context {
		public:

			shared_mux *mux_owner;

			// Garbage Collection Queue:
			std::mutex gc_queue_mutex;
			std::list<mux_signal *> gc_queue;

			~mux_context();
			mux_context(shared_mux *mux_owner);
			mux_context(mux_context const &) = delete;
			mux_context(mux_context &&) = default;
			mux_context &operator=(mux_context const &) = delete;
			mux_context &operator=(mux_context &&) = default;
		};

		mux_context::~mux_context() {
			mux_owner->exit_wait_context(this);
		}

		mux_context::mux_context(shared_mux *mux_owner): mux_owner(mux_owner) {
		}

		mux_signal::~mux_signal() noexcept(false) {
		}

		void mux_signal::clear_timeout() {
			set_timeout(std::chrono::steady_clock::time_point());
		}

		void mux_signal::closed() {
			// Default behavior is to do nothing.
		}

		bool mux_signal::is_timeout_set() const {
			return timeout != std::chrono::steady_clock::time_point();
		}

		void mux_signal::mark_for_close() {
			shared_mux *mo;
			{
				std::lock_guard<std::mutex> sig_lock(mutex);
				if (!mux_owner)
					return;
				mo = mux_owner;
			}
			mo->mark_signal_for_close(this);
		}

		mux_signal::mux_signal(): mux_owner(nullptr) {
		}

		void mux_signal::set_timeout(std::chrono::steady_clock::duration const &timeout) {
			set_timeout(std::chrono::steady_clock::now() + timeout);
		}

		void mux_signal::set_timeout(std::chrono::steady_clock::time_point const &timeout) {
			std::unique_lock<std::mutex> sig_lock(mutex);
			if (mux_owner) {
				mux_owner->set_signal_timeout(this, timeout, sig_lock);
			} else {
				this->timeout = timeout;
				std::atomic_thread_fence(std::memory_order_release);
			}
		}

		void mux_signal::timed_out() {
			// Default behavior is to do nothing.
		}

		void shared_mux::add_signal(std::shared_ptr<mux_signal> const &sig) {

			// signal attributes:
			sig->mux_owner = this;
			sig->mux_stat = mux_signal::mux_state::inactive;
			sig->read_readiness = false;
			sig->write_readiness = false;
			sig->close_readiness = false;
			sig->timeout_readiness = true;
			sig->closing = false;
			sig->gc_cnt = 0;

			{
				std::unique_lock<std::mutex> sig_lock(sig_map_mutex);
				sig_map[sig.get()] = sig;
				if (sig->is_timeout_set())
					set_signal_timeout(sig.get(), sig->timeout, sig_lock);
			}

			// The lock guard, above, ensures a memory barrier so that the signal's
			// attributes (e.g., mux_owner and mux_stat) are visible to the
			// multiplexer threads.

			// add to the epoll object:
			try {
				int flags = sig->initial_readiness();
				epoll_event ev{};
				ev.events = EPOLLET;
				ev.events |= (flags & mux_signal::read_flag) ? EPOLLIN : 0;
				ev.events |= (flags & mux_signal::write_flag) ? EPOLLOUT : 0;
				ev.data.ptr = sig.get();
				int status = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, sig->fd(), &ev);
				if (-1 == status) {
					std::ostringstream ss;
					ss << "error adding file descriptor to epoll object: " << safe_strerror(errno);
					throw std::runtime_error(ss.str());
				}
			} catch (...) {
				{
					std::lock_guard<std::mutex> lock(sig_map_mutex);
					sig_map.erase(sig.get());
				}
				sig->mux_owner = nullptr;
				throw;
			}
		}

		void shared_mux::cancel() {
			{
				std::lock_guard<std::mutex> lock(canceled_mutex);
				canceled = true;
			}
			wake_one();
		}

		std::list<mux_context *>::iterator shared_mux::enter_wait_context(mux_context *ctx) {
			std::lock_guard<std::mutex> wait_ctx_lock(wait_ctx_mutex);
			auto p = wait_ctx_list.insert(wait_ctx_list.end(), ctx);
			return p;
		}

		void shared_mux::exit_wait_context(mux_context *ctx) {
			// search and remove:
			std::lock_guard<std::mutex> wait_ctx_lock(wait_ctx_mutex);
			auto p = std::find(wait_ctx_list.begin(), wait_ctx_list.end(), ctx);
			if (p != wait_ctx_list.end())
				wait_ctx_list.erase(p);
		}

		void shared_mux::exit_wait_context(std::list<mux_context *>::iterator pos) {
			// remove by position:
			std::lock_guard<std::mutex> wait_ctx_lock(wait_ctx_mutex);
			wait_ctx_list.erase(pos);
		}

		void shared_mux::gc_run(mux_context *ctx) {

			// Check the Garbage Collection Queue for this thread. Run collection on
			// any signal that's owned only by this thread.

			std::unique_lock<std::mutex> gc_queue_lock(ctx->gc_queue_mutex);
			while (!ctx->gc_queue.empty()) {

				// pop signal from queue:
				mux_signal *sig = ctx->gc_queue.front();
				ctx->gc_queue.pop_front();
				gc_queue_lock.unlock();
				
				// check for garbage collection:
				int gc_cnt;
				{
					std::lock_guard<std::mutex> sig_lock(sig->mutex);
					--sig->gc_cnt;
					gc_cnt = sig->gc_cnt;
				}
				if (!gc_cnt) {
					gc_signal(sig);
				}

				gc_queue_lock.lock();
			}
		}

		void shared_mux::gc_signal(mux_signal *sig) {

			sig->mux_owner = nullptr;

			// destruct the multiplexer's reference to the signal:
			std::lock_guard<std::mutex> lock(sig_map_mutex);
			sig_map.erase(sig);

			// Releasing the Signal Map Mutex acts as a memory barrier so that the
			// signal attribute, mux_owner, is visible to all threads.
		}

		void shared_mux::mark_signal_for_close(mux_signal *sig) {

			// If this is the first thread to try to mark the signal as closed then
			// mark the signal as Close Ready, ensure the signal is in the Ready
			// Queue, and wake up a multiplexer thread.

			std::unique_lock<std::mutex> sig_lock(sig->mutex);
			if (!sig->close_readiness) {

				// mark:
				sig->close_readiness = true;

				// ensure signal is in Ready Queue:
				if (mux_signal::mux_state::inactive == sig->mux_stat) {
					std::lock_guard<std::mutex> ready_queue_lock(ready_queue_mutex);
					ready_queue.push_back(sig);
					sig->mux_stat = mux_signal::mux_state::queued;
				}
				sig_lock.unlock();

				// wake up a multiplexer thread:
				wake_one();
			}
		}

		void shared_mux::run() {

			// Each multiplexer thread has a unique context.
			mux_context local_ctx(this);

			int to = -1; // epoll timeout

			while (true) {

				// enter Wait state:
				auto wait_ctx_pos = enter_wait_context(&local_ctx);

				// determine epoll timeout period:
				// At most one multiplexer thread will check for a signal timeout, and
				// that thread will check for a timeout of the signal with the soonest
				// timeout time. 
				bool local_timeout_waiting = false;
				if (-1 == to) {
					std::unique_lock<std::mutex> timeout_queue_lock(timeout_queue_mutex);
					mux_signal *sig;
					if (!timeout_queue.empty() && !timeout_waiting) {
						sig = timeout_queue.begin()->second;
						timeout_waiting = true;
						auto sig_timeout = sig->timeout;
						timeout_queue_lock.unlock();
						local_timeout_waiting = true;
						auto now = std::chrono::steady_clock::now();
						if (now >= sig_timeout) {
							to = 0; // timeout has expired--don't block on epoll
						} else {
							to = (sig_timeout - now) / std::chrono::milliseconds(1);
						}
					}
				}

				// wait for an I/O event:
				static int constexpr event_cap = 1;
				epoll_event ev[event_cap];
				int num_events;
				do {
					num_events = epoll_wait(epoll_fd, ev, event_cap, to);
				} while (-1 == num_events && EINTR == errno);
				if (-1 == num_events) {
					std::ostringstream ss;
					ss << "error waiting on epoll object: " << safe_strerror(errno);
					throw std::runtime_error(ss.str());
				}

				// Interpret the I/O events.

				for (int i = 0; i < num_events; ++i) {

					// event object:
					if (!ev[i].data.ptr) {
						// The Cancellation Mutex is needed here because not all event
						// wake-ups signify cancellation. Specifically, if the epoll object
						// wakes up on the event object, and this thread checks the
						// cancellation flag, which is false, but then before clearing the
						// event, another thread cancels the multiplexer (by setting the
						// flag and signaling the event), then this thread may errantly
						// clear the event object and prevent itself and other multiplexer
						// threads from waking up on the event object.
						//
						// It's critical that checking the cancellation flag and clearing the
						// event object are co-atomic.
						std::lock_guard<std::mutex> lock(canceled_mutex);
						if (canceled) {
							wake_one(); // relay event to next thread
							return;
						}
					}

					// signal readiness:
					else {
						mux_signal *sig = static_cast<mux_signal *>(ev[i].data.ptr);
						std::lock_guard<std::mutex> sig_lock(sig->mutex);
						if (sig->mux_stat == mux_signal::mux_state::inactive) {
							std::lock_guard<std::mutex> ready_queue_lock(ready_queue_mutex);
							ready_queue.push_back(sig);
							sig->mux_stat = mux_signal::mux_state::queued;
						}
						if (ev[i].events & EPOLLIN)
							sig->read_readiness = true;
						if (ev[i].events & EPOLLOUT)
							sig->write_readiness = true;
						// FIXME: What to do with EPOLLERR and EPOLLHUP?
					}
				}

				// check for signal timeout:
				{
					auto now = std::chrono::steady_clock::now();
					std::unique_lock<std::mutex> timeout_queue_lock(timeout_queue_mutex);
					if (!timeout_queue.empty()) {
						mux_signal *sig = timeout_queue.begin()->second;
						if (local_timeout_waiting)
							timeout_waiting = false;
						if (now >= sig->timeout) {
							timeout_queue.erase(timeout_queue.begin());
							sig->timeout = std::chrono::steady_clock::time_point(); // reset timeout
							timeout_queue_lock.unlock();
							std::lock_guard<std::mutex> sig_lock(sig->mutex);
							sig->timeout_readiness = true;
							if (sig->mux_stat == mux_signal::mux_state::inactive) {
								std::lock_guard<std::mutex> ready_queue_lock(ready_queue_mutex);
								ready_queue.push_back(sig);
								sig->mux_stat = mux_signal::mux_state::queued;
							}
						}
					}
				}

				// exit Wait state:
				exit_wait_context(wait_ctx_pos);

				// Run garbage collection for this thread.
				gc_run(&local_ctx);

				// Handle some ready signals.

				static int constexpr max_handle_cnt = 32;
				int handle_cnt = 0;
				std::unique_lock<std::mutex> ready_queue_lock(ready_queue_mutex);
				while (handle_cnt < max_handle_cnt && !ready_queue.empty()) {

					// pop signal from queue:
					mux_signal *sig = ready_queue.front();
					ready_queue.pop_front();
					ready_queue_lock.unlock();
					++handle_cnt;

					std::unique_lock<std::mutex> sig_lock(sig->mutex);
					sig->mux_stat = mux_signal::mux_state::in_progress;

					// INVARIANT: The active signal (sig) is not in the Ready Queue and is
					// marked with a state of In Progress.

					// read readiness:
					if (sig->read_readiness && !sig->close_readiness) {
						sig->read_readiness = false;
						sig_lock.unlock();
						mux_signal::ready_result opstat = sig->read_ready();
						if (mux_signal::ready_result::op_incomplete == opstat) {
							sig_lock.lock();
							sig->read_readiness = true;
						} else if (mux_signal::ready_result::signal_complete == opstat) {
							mark_signal_for_close(sig);
							sig_lock.lock();
						} else {
							sig_lock.lock();
						}
					}

					// write readiness:
					if (sig->write_readiness && !sig->close_readiness) {
						sig->write_readiness = false;
						sig_lock.unlock();
						mux_signal::ready_result opstat = sig->write_ready();
						if (mux_signal::ready_result::op_incomplete == opstat) {
							sig_lock.lock();
							sig->write_readiness = true;
						} else if (mux_signal::ready_result::signal_complete == opstat) {
							mark_signal_for_close(sig);
							sig_lock.lock();
						} else {
							sig_lock.lock();
						}
					}

					// timeout:
					if (sig->timeout_readiness && !sig->closing) {
						sig->timeout_readiness = false;
						sig_lock.unlock();
						sig->timed_out();
						sig_lock.lock();
					}

					// close readiness:
					if (sig->close_readiness && !sig->closing) {
						sig->closing = true;
						sig_lock.unlock();
						sig->closed();
						sig_lock.lock();

						// remove file descriptor from epoll object:
						int status = epoll_ctl(epoll_fd, EPOLL_CTL_DEL, sig->fd(), nullptr);
						if (-1 == status) {
							std::ostringstream ss;
							ss << "error removing file descriptor from epoll object: " << safe_strerror(errno);
							throw std::runtime_error(ss.str());
						}

						// Though its file descriptor is removed from the epoll object, the
						// signal's file descriptor is still open and the signal still
						// exists in memory and a reference to it the signal is owned by any
						// multiplexer thread that's in the Wait state. For any thread
						// that's in the Wait state, add the signal to that thread's garbage
						// collection queue. If no such threads exist then destruct the
						// signal reference now.

						{
							std::lock_guard<std::mutex> wait_ctx_lock(wait_ctx_mutex);
							for (auto p = wait_ctx_list.begin(); p != wait_ctx_list.end(); ++p) {
								std::lock_guard<std::mutex> gc_queue_lock((*p)->gc_queue_mutex);
								(*p)->gc_queue.push_back(sig);
								++sig->gc_cnt;
							}
						}
						if (!sig->gc_cnt) {
							sig_lock.unlock();
							gc_signal(sig);
						}

						ready_queue_lock.lock();
						continue;
					}

					// INVARIANT: The current thread holds the signal's mutex.
					//
					// INVARIANT: The active signal is in an In Progress state.
					//
					// INVARIANT: If the active signal is not currently Closing.

					// push signal into Ready Queue if signal is still ready:
					if (sig->read_readiness || sig->write_readiness || sig->timeout_readiness || sig->close_readiness) {
						ready_queue_lock.lock();
						ready_queue.push_back(sig);
						sig->mux_stat = mux_signal::mux_state::queued;
					}

					// otherwise signal is kept out of the Ready Queue and becomes active
					// again only after an epoll wakeup:
					else {
						sig->mux_stat = mux_signal::mux_state::inactive;
						ready_queue_lock.lock();
					}

					// INVARIANT: The current thread holds the Ready Queue Mutex.
				}

				if (!ready_queue.empty())
					to = 0; // don't block on next epoll_wait
				else
					to = -1; // block on next epoll_wait indefinitely
			}

		}

		void shared_mux::set_signal_timeout(mux_signal *sig, std::chrono::steady_clock::time_point const &timeout,
				std::unique_lock<std::mutex> &sig_lock) {

			// Upon calling this function, the signal's mutex is held by the current
			// thread.

			// Remove, add or reorder the signal in the Timeout Queue.
			bool earlier = false;
			{
				std::lock_guard<std::mutex> timeout_queue_lock(timeout_queue_mutex);
				bool was_clear = !sig->is_timeout_set();
				auto prev_timeout = sig->timeout;
				sig->timeout = timeout;
				earlier = timeout_queue.empty() || timeout < timeout_queue.begin()->first;
				if (!was_clear)
					timeout_queue.erase(prev_timeout);
				if (sig->is_timeout_set())
					timeout_queue.insert(std::multimap<std::chrono::steady_clock::time_point, mux_signal *>::value_type(timeout, sig));
			}
			sig_lock.unlock();

			// If the new timeout precedes all existing timeouts then wake up a
			// multiplexer thread so that it can reset its epoll timeout value.
			if (earlier)
				wake_one();
		}

		shared_mux::shared_mux(): canceled{}, timeout_waiting(false) {

			// create epoll object:
			file_descriptor local_epoll_fd = epoll_create1(0);
			if (-1 == local_epoll_fd) {
				std::ostringstream ss;
				ss << "error creating epoll object: " << safe_strerror(errno);
				throw std::runtime_error(ss.str());
			}

			// create control event object, set it to a signaled state, and add it to
			// the epoll object disabled:
			file_descriptor local_ctrl_fd = eventfd(0, 0);
			if (-1 == local_ctrl_fd) {
				std::ostringstream ss;
				ss << "error creating event object: " << safe_strerror(errno);
				throw std::runtime_error(ss.str());
			}
			int status = eventfd_write(local_ctrl_fd, 1);
			if (-1 == status) {
				std::ostringstream ss;
				ss << "error writing to event object: " << safe_strerror(errno);
				throw std::runtime_error(ss.str());
			}
			epoll_event ev{};
			status = epoll_ctl(local_epoll_fd, EPOLL_CTL_ADD, local_ctrl_fd, &ev);
			if (-1 == status) {
				std::ostringstream ss;
				ss << "error adding event object to epoll object: " << safe_strerror(errno);
				throw std::runtime_error(ss.str());
			}

			// done:
			swap(epoll_fd, local_epoll_fd);
			swap(ctrl_fd, local_ctrl_fd);
		}

		void shared_mux::wake_one() {
			// Enable the event object to wake up one multiplexer thread.
			epoll_event ev{};
			ev.events = EPOLLIN | EPOLLONESHOT;
			ev.data.ptr = nullptr;
			int status = epoll_ctl(epoll_fd, EPOLL_CTL_MOD, ctrl_fd, &ev);
			if (-1 == status) {
				std::ostringstream ss;
				ss << "error adding event object to epoll object: " << safe_strerror(errno);
				throw std::runtime_error(ss.str());
			}
		}

	}
}

