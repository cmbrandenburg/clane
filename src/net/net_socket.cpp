// vim: set noet:

/** \file */

#include "net_error.h"
#include "net_socket.h"
#include <assert.h>
#include <errno.h>
#include <poll.h>
#include <sstream>
#include <stdexcept>

namespace clane {
	namespace net {

		accept_result socket::accept(sockaddr *addr, socklen_t *addr_len) const {
			accept_result astat{false, socket(pf, ::accept(fd, addr, addr_len))};
			if (-1 == astat.conn && (EAGAIN == errno || EWOULDBLOCK == errno))
				return astat;
			if (-1 == astat.conn && ECONNABORTED == errno) {
				astat.aborted = true;
				return astat;
			}
			if (-1 == astat.conn) {
				std::ostringstream ss;
				ss << "error accepting connection: " << errno_to_string(errno);
				throw std::runtime_error(ss.str());
			}
			return astat;
		}

		void socket::bind(sockaddr const *addr, socklen_t addr_len) const {
			int status = ::bind(fd, addr, addr_len);
			if (-1 == status) {
				std::ostringstream ss;
				ss << "error binding socket: " << errno_to_string(errno);
				throw std::runtime_error(ss.str());
			}
		}

		void socket::connect(sockaddr const *addr, socklen_t addr_len) const {
			int status;
			while (-1 == (status = ::connect(fd, addr, addr_len)) && EINTR == errno);
			if (-1 == status) {
				std::ostringstream ss;
				ss << "error connecting: " << errno_to_string(errno);
				throw std::runtime_error(ss.str());
			}
		}

		void socket::listen(int backlog) const {
			int status = ::listen(fd, backlog);
			if (-1 == status) {
				std::ostringstream ss;
				ss << "error initiating socket listening: " << errno_to_string(errno);
				throw std::runtime_error(ss.str());
			}
		}

		send_result socket::send_all(void const *buf, size_t cnt) const {
			size_t send_cnt = 0;
			send_result result;
			do {
				result = send(reinterpret_cast<char const *>(buf) + send_cnt, cnt - send_cnt);
				if (result.reset) {
					result.size = send_cnt;
					return result;
				}
				if (!result.size) {
					// blocked: poll for write-readiness
					int status;
					pollfd po{};
					po.fd = *this;
					po.events = POLLOUT;
					while (-1 == (status = poll(&po, 1, -1)) && EINTR == errno);
					if (-1 == status) {
						std::ostringstream ss;
						ss << "error polling socket: " << errno_to_string(errno);
						throw std::runtime_error(ss.str());
					}
				}
				send_cnt += result.size;
			} while (send_cnt < cnt);
			result.size = send_cnt;
			return result;
		}

		void socket::set_reuseaddr() const {
			int arg = 1;
			int status = setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &arg, sizeof(arg));
			if (-1 == status) {
				std::ostringstream ss;
				ss << "error setting SO_REUSEADDR option: " << errno_to_string(errno);
				throw std::runtime_error(ss.str());
			}
		}

		void socket::shut_down(shutdown_how how) const {
			if (-1 == shutdown(*this, static_cast<int>(how))) {
				std::ostringstream ss;
				ss << "error shutting down socket: " << errno_to_string(errno);
				throw std::runtime_error(ss.str());
			}
		}

		socket::socket(protocol_family const *pf, int type, int protocol) : pf(pf) {
			assert(pf);
			while (-1 == (fd = ::socket(pf->domain(), type, protocol)) && EINTR == errno);
			if (-1 == fd) {
				std::ostringstream ss;
				ss << "error creating socket: " << errno_to_string(errno);
				throw std::runtime_error(ss.str());
			}
		}

		socket::socket(protocol_family const *pf, int that_fd) : file_descriptor(that_fd), pf(pf) {
			assert(pf);
		}
	}
}

