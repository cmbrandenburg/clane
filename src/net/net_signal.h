// vim: set noet:

#ifndef CLANE_NET_SIGNAL_H
#define CLANE_NET_SIGNAL_H

/** @file
 *
 * @brief Multiplexer signal types */

#include "net_common.h"
#include "net_mux.h"

namespace clane {
	namespace net {

		// FIXME: unit test
		// FIXME: remove? General-purpose signal timeouts seem to make this class
		// superfluous.
		class mux_timer: public signal {
			file_descriptor fd_;
			std::function<void()> signalee;
		public:
			virtual ~mux_timer() = default;
			mux_timer(std::function<void()> signalee);
			mux_timer(mux_timer const &) = delete;
			mux_timer(mux_timer &&) = default;
			mux_timer &operator=(mux_timer const &) = delete;
			mux_timer &operator=(mux_timer &&) = default;

			void set(std::chrono::steady_clock::duration const &to);

		private:
			virtual file_descriptor const &fd() const;
			virtual ready_result read_ready();
			virtual ready_result write_ready();
		};

		class mux_socket: public signal {
			net::socket sock_;
		protected:
			net::socket const &socket() const { return sock_; }
		public:
			virtual ~mux_socket() = default;
			mux_socket(net::socket &&that_sock);
			mux_socket(mux_socket const &) = delete;;
			mux_socket(mux_socket &&) = default;
			mux_socket &operator=(mux_socket const &) = delete;
			mux_socket &operator=(mux_socket &&) = default;

		private:
			virtual file_descriptor const &fd() const;
		};

		class mux_listener: public mux_socket {
		public:
			virtual ~mux_listener() = default;
			mux_listener(net::socket &&that_sock);
			mux_listener(mux_listener const &) = delete;
			mux_listener(mux_listener &&) = default;
			mux_listener &operator=(mux_listener const &) = delete;
			mux_listener &operator=(mux_listener &&) = default;

		protected:
			struct mux_accept_result {
				bool aborted;
				std::shared_ptr<signal> conn;
			};
			virtual mux_accept_result accept() = 0;

		private:
			virtual int initial_event_flags() const;
			virtual ready_result read_ready();
			virtual ready_result write_ready();
		};

		class mux_conn: public mux_socket {
		protected:
			char *ibuf;
			size_t icap;
			size_t ioffset;
		public:
			virtual ~mux_conn();
			mux_conn(net::socket &&that_sock);
			mux_conn(mux_conn const &) = delete;
			mux_conn(mux_conn &&) = default;
			mux_conn &operator=(mux_conn const &) = delete;
			mux_conn &operator=(mux_conn &&) = default;

		protected:

			void send_all(void const *buf, size_t size);
			void shutdown();

			/** @brief Handles a peer shutdown */
			virtual void finished() = 0;

			/** @brief Handles incoming data
			 *
			 * @return Returns true if and only if the receiver has taken ownership
			 * of the receive buffer. */
			virtual bool recv_some(char *buf, size_t cap, size_t offset, size_t size) = 0;

			// FIXME: Should these be pure virtual?
			virtual void alloc_ibuffer();
			virtual void dealloc_ibuffer();

		private:
			virtual int initial_event_flags() const;
			virtual ready_result read_ready();
		};

		class mux_server_conn: public mux_conn {
		public:
			virtual ~mux_server_conn() = default;
			mux_server_conn(net::socket &&that_sock);
			mux_server_conn(mux_server_conn const &) = delete;
			mux_server_conn(mux_server_conn &&) = default;
			mux_server_conn &operator=(mux_server_conn const &) = delete;
			mux_server_conn &operator=(mux_server_conn &&) = default;

		private:
			virtual ready_result write_ready();
		};
	}
}

#endif // #ifndef CLANE_NET_SIGNAL_H
