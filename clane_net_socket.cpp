// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// vim: set noet:

/** @file */

#include "clane_net_error.hpp"
#include "clane_net_socket.hpp"
#include <sstream>
#include <unistd.h>

namespace clane {
	namespace net {

		posix::unique_fd sys_socket(int domain, int type, int protocol) {
			posix::unique_fd sock = ::socket(domain, type, protocol);
			if (-1 == sock) {
				std::ostringstream ess;
				ess << "socket(domain=" << domain << ", type=" << type << ", protocol=" << protocol << ")";
				throw std::system_error(errno, os_category(), ess.str());
			}
			return sock;
		}

		void sys_setsockopt(int sockfd, int level, int optname, int optval) {
			int stat = ::setsockopt(sockfd, level, optname, &optval, sizeof(optval));
			if (-1 == stat) {
				std::ostringstream ess;
				ess << "setsockopt(sockfd=" << sockfd << ", level=" << level << ", optname=" << optname << ", optval=" << optval << ")";
				throw std::system_error(errno, os_category(), ess.str());
			}
		}

		void sys_bind(int sockfd, sockaddr const *addr, socklen_t addr_len) {
			int stat = ::bind(sockfd, addr, addr_len);
			if (-1 == stat) {
				std::ostringstream ess;
				ess << "bind(sockfd=" << sockfd << ")";
				throw std::system_error(errno, os_category(), ess.str());
			}
		}

		void sys_listen(int sockfd, int backlog) {
			int stat = ::listen(sockfd, backlog);
			if (-1 == stat) {
				std::ostringstream ess;
				ess << "listen(sockfd=" << sockfd << ")";
				throw std::system_error(errno, os_category(), ess.str());
			}
		}

		void sys_getsockname(int sockfd, sockaddr *addr, socklen_t addr_len) {
			socklen_t len = addr_len;
			int stat = ::getsockname(sockfd, addr, &len);
			if (-1 == stat || len > addr_len) {
				std::ostringstream ess;
				ess << "getsockname(sockfd=" << sockfd << ")";
				if (len > addr_len) {
					ess << ": address size is " << len << " bytes, expected no more than " << addr_len << " bytes";
					throw std::logic_error(ess.str());
				}
				throw std::system_error(errno, os_category(), ess.str());
			}
		}

		void sys_getpeername(int sockfd, sockaddr *addr, socklen_t addr_len) {
			socklen_t len = addr_len;
			int stat = ::getpeername(sockfd, addr, &len);
			if (-1 == stat || len > addr_len) {
				std::ostringstream ess;
				ess << "getpeername(sockfd=" << sockfd << ")";
				if (len > addr_len) {
					ess << ": address size is " << len << " bytes, expected no more than " << addr_len << " bytes";
					throw std::logic_error(ess.str());
				}
				throw std::system_error(errno, os_category(), ess.str());
			}
		}

		void sys_connect(int sockfd, sockaddr const *addr, socklen_t addr_len, std::error_code &e) {
			int stat = TEMP_FAILURE_RETRY(::connect(sockfd, addr, addr_len));
			if (-1 == stat) {
				switch (errno) {
					case EACCES:
				 	case EPERM:
					case EAGAIN:
					case ECONNREFUSED:
					case EINPROGRESS:
					case ENETUNREACH:
					case ETIMEDOUT:
						e.assign(errno, os_category());
						break;
					default: {
						std::ostringstream ess;
						ess << "connect(sockfd=" << sockfd << ")";
						throw std::system_error(errno, os_category(), ess.str());
					}
				}
			}
		}

		posix::unique_fd sys_accept(int sockfd, sockaddr *addr, socklen_t addr_len, std::error_code &e) {
			socklen_t sa_len = addr_len;
			posix::unique_fd connfd = TEMP_FAILURE_RETRY(::accept(sockfd, addr, &sa_len));
			if (-1 == connfd) {
				switch (errno) {
					case EAGAIN:
#if EAGAIN != EWOULDBLOCK
					case EWOULDBLOCK:
#endif
					case ECONNABORTED:
					case EMFILE:
				 	case ENFILE:
				 	case ENOBUFS:
				 	case ENOMEM:
					case EPERM:
					case ETIMEDOUT:
						e.assign(errno, os_category());
						break;
					default: {
						std::ostringstream ess;
						ess << "accept(sockfd=" << sockfd << ")";
						throw std::system_error(errno, os_category(), ess.str());
					}
				}
			}
			return connfd;
		}

	}
}

