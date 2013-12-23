// vim: set noet:

#ifndef CLANE_NET_CONN_H
#define CLANE_NET_CONN_H

/** @file
 *
 * @brief Connection and listener base types */

#include "net_common.h"
#include "net_mux.h"
#include "net_socket.h"

namespace clane {
	namespace net {

		class connection: public signal {
			struct send_node {
				send_node *next;
				std::unique_ptr<char> p;
				size_t offset;
				size_t size;
			};
			using signal::owner;
			socket sock;
			bool connected;
			char *ibuf;
			size_t icap;
			size_t ioffset;
			std::mutex send_mutex;
			send_node *send_head;
			send_node *send_tail;
		public:
			virtual ~connection() noexcept;
			connection(socket &&sock): sock(std::move(sock)), connected(true), ibuf{}, send_head{} {}
			connection(connection const &) = delete;
			connection(connection &&) = default;
			connection &operator=(connection const &) = delete;
			connection &operator=(connection &&) = default;
			protocol_family const *protocol() const { return sock.protocol(); }
			std::string local_address() { return sock.local_address(); }
			std::string remote_address() { return sock.remote_address(); }
		protected:
			void set_ibuf(char *p, size_t cap) { ibuf = p; icap = cap; ioffset = 0; }
			status send(void const *p, size_t n);
			void finish();
		private:
			void send_append(char const *p, size_t n);
			virtual int fd() const { return sock.fd(); }
			virtual int initial_event_flags() const { return read_flag | write_flag; }
			virtual ready_result read_ready();
			virtual ready_result write_ready();
		protected:
			virtual void received(char *p, size_t n) = 0;
			virtual void finished() = 0;
			virtual void ialloc() = 0;
			virtual void sent() = 0;
		};

		class listener: public signal {
			socket sock;
		private:
			using signal::owner;
		public:
			virtual ~listener() noexcept = default;
			listener(protocol_family const *pf, char const *addr, int backlog = -1): listener(pf, std::string(addr), backlog) {}
			listener(protocol_family const *pf, std::string addr, int backlog = -1): sock(pf, addr, -1) {}
			listener(listener const &) = delete;
			listener(listener &&) = default;
			listener &operator=(listener const &) = delete;
			listener &operator=(listener &&) = default;
			protocol_family const *protocol() const { return sock.protocol(); }
			std::string address() { return sock.local_address(); }
		private:
			virtual int fd() const { return sock.fd(); }
			virtual int initial_event_flags() const { return read_flag | write_flag; }
			virtual ready_result read_ready();
			virtual ready_result write_ready() { return ready_result::op_complete; }
		protected:
			virtual std::shared_ptr<signal> new_connection(socket &&sock) = 0;
		};
	}
}

#endif // #ifndef CLANE_NET_CONN_H
