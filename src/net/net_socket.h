// vim: set noet:

#ifndef CLANE_NET_SOCKET_H
#define CLANE_NET_SOCKET_H

/** @file
 *
 * @brief Socket */

#include "net_common.h"
#include "net_fd.h"
#include <string>
#include <sys/socket.h>

namespace clane {
	namespace net {

		class accept_result;
		class protocol_family;

		/** @brief Socket receive operation result */
		struct recv_result {

			/** @brief Number of bytes received */
			size_t size;

			/** @brief Whether a connection reset occurred */
			bool reset;

			/** @brief Whether the peer shut down its endpoint for sending */
			bool shutdown;
		};

		/** @brief Options for a receive operation */
		struct recv_options {

			/** @brief Whether to force the receive operation to be nonblocking, even
			 * if the socket is blocking */
			bool nonblocking;
		};

		/** @brief Socket send operation result */
		struct send_result {

			/** @brief Number of bytes received */
			size_t size;

			/** @brief Whether a connection reset occurred */
			bool reset;
		};

		/** @brief Options for a send operation */
		struct send_options {

			/** @brief Whether to force the send operation to be nonblocking, even if
			 * the socket is blocking */
			bool nonblocking;
		};

		/** @brief Low-level socket object */
		class socket: public file_descriptor {
		public:

			/** @brief Connection shutdown directions */
			enum class shutdown_how {
				read = SHUT_RD,
				write = SHUT_WR,
				read_write = SHUT_RDWR
			};

		private:
			protocol_family const *pf;
		public:

			/** @brief Closes the underlying socket */
			~socket() = default;

			/** @brief Constructs by creating an underlying socket */
			socket(protocol_family const *pf, int type, int protocol);

			socket(socket const &) = delete;

			/** @brief Constructs by transferring ownership of `that`'s underlying
			 * socket to `this` */
			socket(socket &&) = default;

			socket &operator=(socket const &) = delete;

			/** @brief Assigns by transferring ownership of `that`'s underlying socket
			 * to `this` */
			socket &operator=(socket &&) = default;

			/** @brief Binds the socket */
			void bind(sockaddr const *addr, socklen_t addr_len) const;

			/** @brief Initiates listening on the socket */
			void listen(int backlog) const;

			/** @brief Accepts an incoming connection on the socket */
			accept_result accept(sockaddr *addr = nullptr, socklen_t *addr_len = nullptr) const;

			/** @brief Accepts an incoming connection on the socket
			 *
			 * @param oaddr If the connection is successfully accepted then `oaddr` is
			 * assigned the remote address of the new connection. */
			accept_result accept(std::string &oaddr) const;

			/** @brief Initiates a connection attempt on the socket */
			void connect(sockaddr const *addr, socklen_t addr_len) const;

			/** @brief Initiates a connection attempt on the socket */
			void connect(std::string const &addr) const;

			/** @brief Initiates a connection attempt on the socket */
			void connect(char const *addr, size_t addr_len) const;

			/** @brief Sets the socket to reuse addresses, if possible */
			void set_reuseaddr() const;

			/** @brief Returns the socket's local address */
			std::string local_addr() const;

			/** @brief Returns the socket's peer's address */
			std::string remote_addr() const;

			/** @brief Receives on a socket */
			recv_result recv(void *buf, size_t cap, recv_options const *opts = nullptr) const;

			/** @brief Sends on a socket */
			send_result send(void const *buf, size_t cnt, send_options const *opts = nullptr) const;

			/** @brief Sends all bytes on a socket
			 *
			 * This blocks as necessary, even if the socket is nonblocking. */
			send_result send_all(void const *buf, size_t cnt) const;

			/** @brief Shuts down one or both halves of a connection */
			void shut_down(shutdown_how how) const;

		private:

			/** @brief Constructs by taking ownership of the `that_fd` file descriptor
			 * */
			socket(protocol_family const *pf, int that_fd);
		};

		/** @brief Socket accept operation result */
		struct accept_result {

			/** @brief Whether the accept operation aborted */
			bool aborted;

			/** @brief Accepted connection, if any */
			socket conn;
		};

		/** @brief Socket protocol family */
		class protocol_family {
		public:
			~protocol_family() = default;
			protocol_family() = default;
			protocol_family(protocol_family const &) = delete;
			protocol_family(protocol_family &&) = default;
			protocol_family &operator=(protocol_family const &) = delete;
			protocol_family &operator=(protocol_family &&) = default;

			/** @brief Returns the domain (e.g., `AF_INET`) */
			virtual int domain() const = 0;

			/** @brief Returns the local address of the socket
			 *
			 * A socket has a local address if it's connected or has been bound. */
			virtual std::string local_addr(socket const &sock) const = 0;

			/** @brief Returns the remote address of the socket
			 *
			 * A socket has a remote address if and only if it's connected */
			virtual std::string remote_addr(socket const &sock) const = 0;

			/** @brief Accepts a connection on a socket and gets the connection's
			 * remote address */
			virtual accept_result accept(socket const &lis, std::string &oname) const = 0;

			/** @brief Initiates a connection attempt on a socket */
			virtual void connect(socket const &conn, char const *addr, size_t addr_len) const = 0;

			/** @brief Receives on a socket */
			virtual recv_result recv(socket const &conn, void *buf, size_t cap, recv_options const *opts) const = 0;

			/** @brief Sends on a socket */
			virtual send_result send(socket const &conn, void const *buf, size_t cnt, send_options const *opts) const = 0;
		};

		inline accept_result socket::accept(std::string &oaddr) const {
			return pf->accept(*this, oaddr);
		}

		inline void socket::connect(std::string const &addr) const {
			connect(addr.c_str(), addr.size());
		}

		inline void socket::connect(char const *addr, size_t addr_len) const {
			pf->connect(*this, addr, addr_len);
		}

		inline std::string socket::local_addr() const {
			return pf->local_addr(*this);
		}

		inline recv_result socket::recv(void *buf, size_t cap, recv_options const *opts) const {
			return pf->recv(*this, buf, cap, opts);
		}

		inline std::string socket::remote_addr() const {
			return pf->remote_addr(*this);
		}

		inline send_result socket::send(void const *buf, size_t cnt, send_options const *opts) const {
			return pf->send(*this, buf, cnt, opts);
		}
	}
}

#endif // #ifndef CLANE_NET_SOCKET_H
