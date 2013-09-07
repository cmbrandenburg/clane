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
#include <functional>
#include <list>
#include <map>
#include <memory>
#include <mutex>

namespace clane {
	namespace net {

		class mux_context;
		class mux_signal;

		/** @brief Multi-threaded I/O and event multiplexer */
		class shared_mux {
			friend class mux_context;
			friend class mux_signal;

			file_descriptor epoll_fd;
			file_descriptor ctrl_fd;

			std::mutex canceled_mutex;
			bool canceled;

			// Signal Map:
			std::mutex sig_map_mutex;
			std::map<mux_signal *, std::shared_ptr<mux_signal>> sig_map;

			// Ready Queue:
			std::mutex ready_queue_mutex;
			std::list<mux_signal *> ready_queue;

			// Timeout Queue:
			std::mutex timeout_queue_mutex;
			bool timeout_waiting;
			std::multimap<std::chrono::steady_clock::time_point, mux_signal *> timeout_queue;

			// Wait Context List: all contexts currently in the Wait state
			std::mutex wait_ctx_mutex;
			std::list<mux_context *> wait_ctx_list;

		public:

			~shared_mux() = default;

			/** @brief Constructs an empty multiplexer */
			shared_mux();

			shared_mux(shared_mux const &) = delete;
			shared_mux(shared_mux &&) = default;
			shared_mux &operator=(shared_mux const &) = delete;
			shared_mux &operator=(shared_mux &&) = default;

			/** @brief Adds a signal to the multiplexer
			 *
			 * A signal may be added to a multiplexer at any time, though a signal may
			 * be added no more than once. Once a signal is added to a multiplexer,
			 * the multiplexer will drive I/O and events for that signal.
			 *
			 * The multiplexer owns a shared reference to the added signal. The signal
			 * will remain owned by the multiplexer for the lifetime of the
			 * signal—i.e., until the signal is closed. */
			void add_signal(std::shared_ptr<mux_signal> const &sig);

			/** @brief Closes a signal, eventually
			 *
			 * A signal is closed asynchronously by first marking it for closure, and
			 * then, possibly in another thread context, closing it. */
			void mark_signal_for_close(mux_signal *sig);

			/** @brief Runs the multiplexer in the current thread
			 *
			 * In order to drive I/O events, the multiplexer must run in at least one
			 * thread. Once it is running in a thread, the multiplexer will continue
			 * running in that thread until the multiplexer is canceled.
			 *
			 * The multiplexer may be run in multiple threads simultaneously. With
			 * such use, the multiplexer will drive I/O from all such threads,
			 * splitting the load between them. */
			void run();

			/** @brief Stops the multiplexer
			 *
			 * Once a multiplexer is canceled, it cannot be restarted. */
			void cancel();

		private:
			void wake_one();

			void gc_run(mux_context *ctx);
			void gc_signal(mux_signal *sig);

			std::list<mux_context *>::iterator enter_wait_context(mux_context *ctx);
			void exit_wait_context(mux_context *ctx);
			void exit_wait_context(std::list<mux_context *>::iterator pos);

			void set_signal_timeout(mux_signal *sig, std::chrono::steady_clock::time_point const &timeout,
					std::unique_lock<std::mutex> &sig_lock);
		};

		/** @brief Single-threaded I/O and event multiplexer
		 *
		 * The single-threaded multiplexer is currently defined to be a
		 * multi-threaded multiplexer. This is a temporary development hack.
		 * Eventually, the shared_mux will be implemented separately and optimized
		 * for the single-threaded case. */
		typedef shared_mux unique_mux;

		/** @brief Base class for all multiplexer signals */
		class mux_signal {
			friend class shared_mux;

			enum class mux_state: char {
				inactive,
				queued, // in the multiplexer's Ready Queue
				in_progress
			};

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

				/** @brief Signal is finished handling I/O-readiness
				 *
				 * After a signal returns the `signal_complete` value, the multiplexer
				 * will stop detecting I/O-readiness events for the signal and destroy
				 * its reference to the signal—thus causing the signal to close when all
				 * other references have been destroyed. */
				signal_complete
			};

			/** @brief Readiness flags */
			enum {

				/** @brief Signifies Read Readiness */
				read_flag = 1<<0,

				/** @brief Signified Write Readiness */
				write_flag = 1<<1
			};

		private:

			std::mutex mutex;

		protected:

			/** @brief The multiplexer that owns this signal
			 *
			 * @note The `mux_owner` member variable is safe to access in a derived
			 * classes only while the signal is guaranteed not to be closed. An
			 * example of when this guarantee exists is while handling a multiplexer
			 * event, such as in the `read_ready` function. Owning a shared reference
			 * to the signal is not enough to guarantee safe access. */
			shared_mux *mux_owner;

		private:

			// readiness:
			mux_state mux_stat;
			bool read_readiness;
			bool write_readiness;
			bool close_readiness;
			bool timeout_readiness;
			bool closing;

			// garbage collection:
			int gc_cnt;

			// timeout:
			std::chrono::steady_clock::time_point timeout;
			bool waited_on;

		public:
			virtual ~mux_signal() noexcept(false);

			/** @brief Constructs an unowned signal with no timeout */
			mux_signal();

			mux_signal(mux_signal const &) = delete;
			mux_signal(mux_signal &&) = default;
			mux_signal &operator=(mux_signal const &) = delete;
			mux_signal &operator=(mux_signal &&) = default;

		protected:

			/** @brief Marks the signal as ready to be closed */
			void mark_for_close();

			/** @brief Clears the signal's timeout */
			void clear_timeout();

			/** @brief Sets the timeout relative to the current time */
			void set_timeout(std::chrono::steady_clock::duration const &timeout);

			/** @brief Sets the timeout as an absolute time
			 *
			 * A signal may have at most one timeout associated with it. When that
			 * timeout expires, the signal's `timed_out` function is called by the
			 * multiplexer. Upon expiration, the signal's timeout is cleared. */
			void set_timeout(std::chrono::steady_clock::time_point const &timeout);

			/** @brief Returns the signal's underlying file descriptor
			 *
			 * @par Availability:
			 * Linux */
			virtual file_descriptor const &fd() const = 0;

			/** @brief Returns the signal's initial readiness
			 *
			 * @par Availability:
			 * Linux */
			virtual int initial_readiness() const = 0;

			/** @brief Handles a Read Readiness event for the signal */
			virtual ready_result read_ready() = 0;

			/** @brief Handles a Write Readiness event for the signal */
			virtual ready_result write_ready() = 0;

			/** @brief Handles a Timeout for the signal
			 *
			 * The default implementation does nothing */
			virtual void timed_out();

			/** @brief Handles a Close Event for the signal
			 *
			 * Once closed, the multiplexer relinquishes its shared reference to the
			 * signal, and the signal will receive no subsequent events. However, the
			 * signal will remain in existence until all shared references are
			 * destructed. */
			virtual void closed();

		private:

			bool is_timeout_set() const;
		};

	}
}

#endif // #ifndef CLANE_NET_MUX_H
