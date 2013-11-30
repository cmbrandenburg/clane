// vim: set noet:

#ifndef CLANE_NET_MUX_H
#define CLANE_NET_MUX_H

/** @file
 *
 * @brief I/O multiplexing */

#include "net_common.h"
#include "net_socket.h"
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <list>
#include <map>
#include <memory>
#include <mutex>

namespace clane {
	namespace net {

		class signal;

		/** @brief Base class for all multiplexers
		 *
		 * A multiplexer drives I/O for zero or more attached signals (see @ref
		 * signal and attach_signal()). */
		class mux {
			friend class signal;
		public:
			virtual ~mux() = default;
			mux() = default;
			mux(mux const &) = delete;
			mux(mux &&) = delete;
			mux &operator=(mux const &) = delete;
			mux &operator=(mux &&) = delete;

			/** @brief Attaches a signal to the multiplexer
			 *
			 * Attaching a signal to a multiplexer causes the multiplexer to own a
			 * shared reference to that signal and to drive the signal's I/O events.
			 * The signal handles its I/O events via `virtual` member functions.
			 *
			 * A signal that's attached to a multiplexer remains attached until either
			 * it's explicitly detached (via detach_signal()) or else the multiplexer
			 * terminates. When a signal detaches from a multiplexer, the multiplexer
			 * relinquishes its shared reference to the signal. This may cause the
			 * signal to destruct.
			 *
			 * An unattached signal may attach to a multiplexer at any time during the
			 * lifetime of the multiplexer, including the time before the multiplexer
			 * runs. If the signal attaches after the multiplexer terminates then
			 * there is no effect: the signal remains unattached and no error occurs.
			 *
			 * A signal may be attached to at most one multiplexer at any given time.
			 *
			 * @sa detach_signal(), run(), terminate() */
			virtual void attach_signal(std::shared_ptr<signal> const &sig) = 0;

			/** @brief Detaches a signal from the multiplexer
			 *
			 * When a signal detaches from its multiplexer, the multiplexer
			 * relinquishes its shared reference to the signal and stops driving the
			 * signal's I/O events.
			 *
			 * A signal may detach from a multiplexer only while the signal is
			 * attached to that multiplexer. Multi-threaded applications may need to
			 * synchronize detachment so that a signal doesn't attempt to detach
			 * twice.
			 *
			 * The detach_signal() function initiates detachment, possibly returning
			 * before the signal is detached. If the multiplexer owns the sole
			 * reference to the signal then the signal destructs.
			 *
			 * @sa attach_signal() */
			virtual void detach_signal(signal *sig) = 0;

			/** @brief Runs the multiplexer in the current thread
			 *
			 * Running a multiplexer causes it to continually drive I/O events for all
			 * attached signals until the multiplexer terminates (via terminate()) or
			 * else a nonrecoverable error occurs. Once a multiplexer stops running,
			 * it can't run again; its only remaining action is destruction.
			 *
			 * A multiplexer may run in multiple concurrent threads only if the
			 * multiplexer type supports doing so. Multi-threaded multiplexers handle
			 * multiple I/O events simultaneously—one event in each thread—though a
			 * signal's events are handled in at most one thread at any time.
			 *
			 * The run() function returns when the multiplexer has terminated.
			 *
			 * @sa attach_signal(), detach_signal(), terminate() */
			virtual void run() = 0;

			/** @brief Terminates the multiplexer
			 *
			 * Termination causes a multiplexer to detach all of its signals and to
			 * have all invocations of its run() function return. The terminate()
			 * function may return before these conditions are met. Use the wait()
			 * function to wait until termination has completed.
			 *
			 * @sa run(), wait() */
			virtual void terminate() = 0;

			/** @brief Waits for the multiplexer's termination to complete
			 *
			 * The wait() function waits for the multiplexer to detach all of its
			 * signals and have all invocations of its run() function either return or
			 * be in an imminently returnable state.
			 *
			 * @sa terminate() */
			virtual void wait() = 0;

		private:

			// Sets or clears an attached signal's timer, and ensures that the
			// multiplexer's timeout state is updated as needed.
			virtual void set_signal_timer(signal *sig, std::chrono::steady_clock::time_point const &new_timeout) = 0;
		};

		/** @brief Multi-threaded multiplexer
		 *
		 * @sa smux */
		class mmux: public mux {
			friend class wait_context;

			// Underlying entry in the Signal Map for an attached signal.
			class sig_entry {
			public:
				std::shared_ptr<signal> sig;
				// synchronized by the Ready Mutex:
			private:
				enum {
					inactive,
					queued, // in the multiplexer's Ready Queue
					in_progress
				} stat;
			public:
				bool read_readiness;
				bool write_readiness;
				bool timeout_readiness;
				bool detach_readiness;
				// synchronized by the Garbage Collection Mutex:
				int gc_cnt;
			public:
				~sig_entry() = default;
				sig_entry(std::shared_ptr<signal> const &sig);
				sig_entry(sig_entry const &) = delete;
				sig_entry(sig_entry &&) = default;
				sig_entry &operator=(sig_entry const &) = delete;
				sig_entry &operator=(sig_entry &&) = default;
				bool is_waiting() const { return inactive == stat; }
				void set_as_waiting() { stat = inactive; }
				void set_as_queued() { stat = queued; }
				void set_as_in_progress() { stat = in_progress; }
			};

			// Represents a multiplexer thread.
			class wait_context {
			public:
				mmux *owner;
				bool waiting;
				std::list<sig_entry *>::const_iterator gc_queue_pos;
			public:
				~wait_context();
				wait_context(mmux *owner);
				wait_context(wait_context const &) = delete;
				wait_context(wait_context &&) = default;
				wait_context &operator=(wait_context const &) = delete;
				wait_context &operator=(wait_context &&) = default;
			};

			file_descriptor epoll_fd;
			file_descriptor ctrl_fd;

			std::mutex term_mutex;
			int thrd_cnt;
			bool term_start;
			std::condition_variable term_cond;

			// Signal Map:
			std::mutex sig_map_mutex;
			std::map<sig_entry *, std::unique_ptr<sig_entry>> sig_map;

			// Ready Queue:
			std::mutex ready_mutex;
			std::list<sig_entry *> ready_queue;

			// Timeout Queue:
			std::mutex timeout_mutex;
			bool timeout_waiting;
			std::multimap<std::chrono::steady_clock::time_point, sig_entry *> timeout_queue;

			// Wait State List and Garbage Collection Queue:
			std::mutex gc_mutex; // synchronizes both containers
			std::list<wait_context *> wait_ctx_list;
			std::list<sig_entry *> gc_queue;

		public:

			~mmux() noexcept;

			/** @brief Constructs an empty multiplexer */
			mmux();

			mmux(mmux const &) = delete;
			mmux(mmux &&) = delete;
			mmux &operator=(mmux const &) = delete;
			mmux &operator=(mmux &&) = delete;

			virtual void attach_signal(std::shared_ptr<signal> const &sig);
			virtual void detach_signal(signal *sig);
			virtual void run();
			virtual void terminate();
			virtual void wait();

		private:
			virtual void set_signal_timer(signal *sig, std::chrono::steady_clock::time_point const &new_timeout);

			void wake_one();
			void detach_signal(sig_entry *ent);
			void detach_signal_entry(sig_entry *ent); // performs detachment, unlike detach_signal(), which marks as Detach Ready

			// Wait State & signal garbage collection:
			void release_signal(sig_entry *ent);

			// XXX: All queues' operations are specified as noexcept because
			// eventually they will never throw exceptions. For now, however,
			// we're using standard library containers, which may throw.

			// Ready Queue operations:
			//
			// The current thread must hold the Ready Queue Mutex when calling
			// any of these functions.
			//
			bool ready_queue_empty() const noexcept;
			void ready_queue_push(sig_entry *ent) noexcept;
			sig_entry *ready_queue_pop() noexcept;

			// Timeout Queue operations:
			//
			// The current thread must hold the Timeout Queue Mutex when calling
			// any of these functions.
			//
			bool timeout_queue_empty() const noexcept;
			void timeout_queue_pop_front() noexcept;
			sig_entry *timeout_queue_front() const noexcept;
			void timeout_queue_erase(sig_entry *ent) noexcept;
			void timeout_queue_insert(sig_entry *ent) noexcept;

		};

		/** @brief Single-threaded multiplexer
		 *
		 * The single-threaded multiplexer is type-defined as a multi-threaded
		 * multiplexer. This is temporary. Eventually, the @ref smux will be
		 * implemented separately and optimized for the single-threaded case. */
		typedef mmux smux;

		/** @brief Base class for all multiplexer signals
		 *
		 * A signal is an I/O object that is (optionally) driven by a multiplexer.
		 * The multiplexer notifies the signal of I/O events via the signal's
		 * `virtual` member functions. The events are platform-specific.
		 *
		 * @sa mux */
		class signal {
			friend class mmux;

		protected:

			/** @brief Type for signifying the result of an I/O-readiness operation */
			enum class ready_result {

				/** @brief Signal has more data to process
				 *
				 * After a signal returns the `op_incomplete` value, the multiplexer
				 * will call the same I/O-readiness function again to allow the signal
				 * to try to complete I/O. */
				op_incomplete,

				/** @brief Signal has no more data to process
				 *
				 * After a signal returns the `op_complete` value, the multiplexer will
				 * _not_ call the same I/O-readiness function again until I/O-readiness
				 * has been triggered by the operating system. */
				op_complete,

				/** @brief Signal is ready to detach
				 *
				 * After a signal returns the `signal_complete` value, the multiplexer
				 * will detach the signal. */
				signal_complete
			};

			/** @brief Event flags */
			enum {

				/** @brief Signifies Read Readiness */
				read_flag = 1<<0,

				/** @brief Signified Write Readiness */
				write_flag = 1<<1
			};

		protected:

			/** @brief The multiplexer that owns this signal
			 *
			 * @note The `owner` member variable is safe to access in a derived
			 * classes only while the signal is guaranteed not to be detached.
			 * An example of when this guarantee exists is while handling a
			 * multiplexer event, such as in the `read_ready` function. Owning a
			 * shared reference to the signal is not enough to guarantee safe
			 * access. */
			mux *owner;

		private:
			void *ent; // controlled by multiplexer
			std::chrono::steady_clock::time_point timeout;

		public:
			virtual ~signal() noexcept(false);

			/** @brief Constructs an unattached signal with no timeout */
			signal();

			signal(signal const &) = delete;
			signal(signal &&) = default;
			signal &operator=(signal const &) = delete;
			signal &operator=(signal &&) = default;

		protected:

			/** @brief Clears the signal's timer
			 *
			 * If a signal's timer is clear then the signal will not receive a timeout
			 * event.
			 *
			 * @sa set_timer(std::chrono::steady_clock::time_point const &) */
			void clear_timer();

			/** @brief Sets the signal's timer value as a relative time
			 *
			 * @sa clear_timer(), set_timer(std::chrono::steady_clock::time_point
			 * const &) */
			void set_timer(std::chrono::steady_clock::duration const &timeout);

			/** @brief Sets the signal's timer value as an absolute time
			 *
			 * If a signal has its timer set and the signal is attached to a
			 * multiplexer and the timer expires then the multiplexer will notify the
			 * signal of the timeout event via the timed_out() virtual member
			 * function.
			 *
			 * If a signal's timer expires while the signal is unattached to any
			 * multiplexer then no timeout event will occur but the timer will remain
			 * in an expired state. If the signal is later attached to a multiplexer
			 * then the multiplexer will imminently notify the signal of its timeout
			 * event.
			 *
			 * A signal's timer may be set at any time, including when the signal is
			 * unattached to any multiplexer, but the act of setting a signal's timer
			 * is _not_ synchronized with the signal becoming attached to or detached
			 * from a multiplexer. Because of this fact, and because an attached
			 * signal may become detached at any time other than when the signal is
			 * handling an I/O event, the act of setting a signal's timer is
			 * inherently safe only when (1) the signal is detached or (2) the signal
			 * is handling an I/O event. Otherwise, if a signal is attached and not
			 * handling an I/O event, then explicit synchronization is needed.
			 *
			 * A signal has only one timer.
			 *
			 * Immediately before a signal receives its timeout event, the signal's
			 * timer is cleared.
			 *
			 * @sa clear_timer(), set_timer(std::chrono::steady_clock::duration const
			 * &) */
			void set_timer(std::chrono::steady_clock::time_point const &timeout);

			/** @brief Detaches the signal from its multiplexer
			 *
			 * The detach() function behaves as if calling mux::detach_signal() using
			 * the signal's multiplexer and the signal as arguments.
			 *
			 * @sa mux::attach_signal(), mux::detach_signal() */
			void detach();

			/** @brief Returns the signal's underlying file descriptor
			 *
			 * @par Availability
			 * Linux */
			virtual file_descriptor const &fd() const = 0;

			/** @brief Returns the signal's initial event flags
			 *
			 * A signal's initial event flags determine which events, if any, the
			 * signal's multiplexer will detect.
			 *
			 * @par Availability
			 * Linux */
			virtual int initial_event_flags() const = 0;

			/** @brief Handles a Read Readiness event for the signal */
			virtual ready_result read_ready() = 0;

			/** @brief Handles a Write Readiness event for the signal */
			virtual ready_result write_ready() = 0;

			/** @brief Handles a Timeout for the signal
			 *
			 * The default implementation does nothing */
			virtual void timed_out();

			/** @brief Handles a <em>Pre-detachment Event</em> for the signal
			 *
			 * A <em>Pre-detachment Event</em> occurs imminently before a signal
			 * becomes detached from its multiplexer. During the event, the signal is
			 * still attached.
			 *
			 * @sa detach(), detached() */
			virtual void detaching();

			/** @brief Handles a <em>Post-detachment Event</em> for the signal
			 *
			 * A <em>Post-detachment Event</em> occurs imminently after a signal has
			 * detached from its multiplexer.
			 *
			 * @sa detach(), detaching() */
			virtual void detached();

		private:
			void reset_timeout() { timeout = std::chrono::steady_clock::time_point(); }
			bool is_timeout_set() const { return timeout != std::chrono::steady_clock::time_point(); }
		};

	}
}

#endif // #ifndef CLANE_NET_MUX_H
