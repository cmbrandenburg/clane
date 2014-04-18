// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// vim: set noet:

/** @file */

#include "clane_net_error.hpp"
#include "clane_net_inet.hpp"
#include "clane_net_socket.hpp"
#include <arpa/inet.h>
#include <cassert>
#include <netdb.h>
#include <netinet/ip.h>
#include <sstream>
#include <unistd.h>

namespace clane {
	namespace net {

		protocol_family const *tcp_protocol_family_by_domain(int domain) {
			switch (domain) {
				case AF_INET: return &tcp4;
				case AF_INET6: return &tcp6;
				default: assert(false); return nullptr;
			}
		}

		std::string tcp4_address_to_string(sockaddr_in const &sa) {
			char buf[INET_ADDRSTRLEN];
			char const *addr = inet_ntop(AF_INET, &sa.sin_addr, buf, sizeof(buf));
			if (!addr)
				throw std::system_error(errno, os_category(), "inet_ntop");
			std::ostringstream ss;
			ss << addr << ':' << be16toh(sa.sin_port);
			return ss.str();
		}

		std::string tcp6_address_to_string(sockaddr_in6 const &sa) {
			char buf[INET6_ADDRSTRLEN];
			char const *addr = inet_ntop(AF_INET6, &sa.sin6_addr, buf, sizeof(buf));
			if (!addr)
				throw std::system_error(errno, os_category(), "inet_ntop");
			std::ostringstream ss;
			ss << '[' << addr << "]:" << be16toh(sa.sin6_port);
			return ss.str();
		}

		class unique_addrinfo {
			addrinfo *ai;
		public:
			~unique_addrinfo() noexcept { if (ai) { freeaddrinfo(ai); }}
			unique_addrinfo(addrinfo *ai) noexcept: ai(ai) {}
			unique_addrinfo(unique_addrinfo const &) = delete;
			unique_addrinfo(unique_addrinfo &&that) noexcept: ai{} { swap(that); }
			unique_addrinfo &operator=(unique_addrinfo const &) = delete;
			unique_addrinfo &operator=(unique_addrinfo &&that) noexcept { swap(that); return *this; }
			addrinfo *operator->() { return ai; }
			addrinfo const *operator->() const { return ai; }
			void swap(unique_addrinfo &that) noexcept { std::swap(ai, that.ai); }
		};

		unique_addrinfo resolve_inet_address(int domain, int type, bool passive, std::string &addr) {
			// split address host and port:
			std::string::size_type host{}, port, sep;
			char saved_sep;
			if (addr.size() && addr[0] == '[' && (addr.npos != (sep = addr.find(']', 1))) && addr.size() > sep && addr[sep+1] == ':') {
				// IPv6 literal
				++host;
				port = sep + 2;
			} else if (addr.npos != (sep = addr.find(':', host))) {
				port = sep + 1;
			} else {
				std::ostringstream ess;
				ess << "missing address-port separator (':') in address \"" << addr << '"';
				throw std::invalid_argument(ess.str());
			}
			saved_sep = addr[sep];
			addr[sep] = 0; // null terminator for host
			// resolve:
			addrinfo hints{};
			hints.ai_flags = AI_ADDRCONFIG | (passive ? AI_PASSIVE : 0);
			hints.ai_family = domain;
			hints.ai_socktype = type;
			hints.ai_protocol = 0;
			addrinfo *results{};
			// FIXME: Does getaddrinfo ever interrupt due to EINTR?
			char const *node = host == sep ? nullptr : addr.data() + host;
			int stat = ::getaddrinfo(node, addr.c_str() + port, &hints, &results);
			if (stat) {
				addr[sep] = saved_sep; // restore address
				if (EAI_SYSTEM == stat)
					throw std::system_error(errno, os_category(), "getaddrinfo");
				std::ostringstream ess;
				ess << "getaddrinfo: " << gai_strerror(stat);
				throw std::runtime_error(ess.str());
			}
			return results;
		}

		// = PROTOCOL FAMILY METHODS =

		void pf_tcpx_construct_descriptor(socket_descriptor &sd) {
			sd.n = -1;
		}

		void pf_tcpx_destruct_descriptor(socket_descriptor &sd) {
			if (-1 != sd.n) {
				posix::close_fd(sd.n);
			}
		}

		int pf_tcpx_descriptor(socket_descriptor const &sd) {
			return sd.n;
		}

		socket pf_tcp_new_listener(int domain, std::string &addr, int backlog) {
			auto lookup = resolve_inet_address(domain, SOCK_STREAM, true, addr);
			auto sock_fd = sys_socket(lookup->ai_family, SOCK_STREAM | SOCK_NONBLOCK, 0);
			sys_setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, 1);
			sys_bind(sock_fd, lookup->ai_addr, lookup->ai_addrlen);
			sys_listen(sock_fd, backlog < 0 ? 256 : backlog);
			return socket(tcp_protocol_family_by_domain(lookup->ai_family), std::move(sock_fd));
		}

		socket pf_tcpx_new_listener(std::string &addr, int backlog) { return pf_tcp_new_listener(AF_UNSPEC, addr, backlog); }
		socket pf_tcp4_new_listener(std::string &addr, int backlog) { return pf_tcp_new_listener(AF_INET, addr, backlog); }
		socket pf_tcp6_new_listener(std::string &addr, int backlog) { return pf_tcp_new_listener(AF_INET6, addr, backlog); }

		socket pf_tcp_new_connection(int domain, std::string &addr, std::error_code &e) {
			auto lookup = resolve_inet_address(domain, SOCK_STREAM, false, addr);
			auto sock_fd = sys_socket(lookup->ai_family, SOCK_STREAM, 0);
			socket sock;
			std::error_code e2;
			sys_connect(sock_fd, lookup->ai_addr, lookup->ai_addrlen, e2);
			if (e2 && e2 != std::make_error_condition(std::errc::operation_in_progress)) {
				e = e2;
				return sock;
			}
			sock = socket(tcp_protocol_family_by_domain(lookup->ai_family), std::move(sock_fd));
			return sock;
		}

		socket pf_tcpx_new_connection(std::string &addr, std::error_code &e) {
		 	return pf_tcp_new_connection(AF_UNSPEC, addr, e);
	 	}

		socket pf_tcp4_new_connection(std::string &addr, std::error_code &e) {
		 	return pf_tcp_new_connection(AF_INET, addr, e);
	 	}

		socket pf_tcp6_new_connection(std::string &addr, std::error_code &e) {
		 	return pf_tcp_new_connection(AF_INET6, addr, e);
	 	}

		void pf_tcpx_set_nonblocking(socket_descriptor &sd) {
			posix::set_nonblocking(sd.n);
		}

		std::string pf_tcp4_local_address(socket_descriptor &sd) {
			sockaddr_in sa;
			sys_getsockname(sd.n, reinterpret_cast<sockaddr *>(&sa), sizeof(sa));
			return tcp4_address_to_string(sa);
		}

		std::string pf_tcp4_remote_address(socket_descriptor &sd) {
			sockaddr_in sa;
			sys_getpeername(sd.n, reinterpret_cast<sockaddr *>(&sa), sizeof(sa));
			return tcp4_address_to_string(sa);
		}

		std::string pf_tcp6_local_address(socket_descriptor &sd) {
			sockaddr_in6 sa;
			sys_getsockname(sd.n, reinterpret_cast<sockaddr *>(&sa), sizeof(sa));
			return tcp6_address_to_string(sa);
		}

		std::string pf_tcp6_remote_address(socket_descriptor &sd) {
			sockaddr_in6 sa;
			sys_getpeername(sd.n, reinterpret_cast<sockaddr *>(&sa), sizeof(sa));
			return tcp6_address_to_string(sa);
		}

		template <class SockAddr, protocol_family const *pf, std::string (*addr_to_str)(SockAddr const &)>
		socket pf_tcp_accept(socket_descriptor &sd, std::string *addr_o, std::error_code &e) {
			SockAddr sa;
			socket sock;
			auto conn_fd = sys_accept(sd.n, reinterpret_cast<sockaddr *>(addr_o ? &sa : nullptr), sizeof(sa), e);
			if (-1 == conn_fd)
				return sock;
			sock = socket(pf, std::move(conn_fd));
			if (addr_o)
				*addr_o = addr_to_str(sa);
			return sock;
		}

		socket pf_tcp4_accept(socket_descriptor &sd, std::string *addr_o, std::error_code &e) {
			return pf_tcp_accept<sockaddr_in, &tcp4, tcp4_address_to_string>(sd, addr_o, e);
		}

		socket pf_tcp6_accept(socket_descriptor &sd, std::string *addr_o, std::error_code &e) {
			return pf_tcp_accept<sockaddr_in6, &tcp6, tcp6_address_to_string>(sd, addr_o, e);
		}

		size_t pf_tcpx_send(socket_descriptor &sd, void const *p, size_t n, int flags, std::error_code &e) {
			char const *const ppos = reinterpret_cast<char const *>(p);
			size_t tot = 0;
			do {
				ssize_t stat = TEMP_FAILURE_RETRY(::send(sd.n, ppos+tot, n-tot, MSG_NOSIGNAL));
				if (-1 == stat) {
					switch (errno) {
						case EACCES:
						case EAGAIN:
#if EAGAIN != EWOULDBLOCK
						case EWOULDBLOCK:
#endif
						case ECONNRESET:
					 	case EPIPE:
						case ENOBUFS:
						case ETIMEDOUT:
							e.assign(errno, os_category());
							return 0;
						default:
							throw std::system_error(errno, os_category(), "send");
					}
				}
				tot += stat;
			} while (flags & all && tot < n);
			return tot;
		}

		size_t pf_tcpx_recv(socket_descriptor &sd, void *p, size_t n, int flags, std::error_code &e) {
			char *const ppos = reinterpret_cast<char *>(p);
			size_t tot = 0;
			do {
				ssize_t stat = TEMP_FAILURE_RETRY(::recv(sd.n, ppos+tot, n-tot, 0));
				if (-1 == stat) {
					switch (errno) {
						case EACCES:
						case EAGAIN:
#if EAGAIN != EWOULDBLOCK
						case EWOULDBLOCK:
#endif
						case ECONNREFUSED:
						case ECONNRESET:
						case ENOMEM:
						case ETIMEDOUT:
							e.assign(errno, os_category());
							return 0;
						default:
							throw std::system_error(errno, os_category(), "recv");
					}
				}
				tot += stat;
			} while (flags & all && tot < n);
			return tot;
		}

		void pf_tcpx_fin(socket_descriptor &sd) {
			int stat = ::shutdown(sd.n, SHUT_WR);
			if (-1 == stat)
				throw std::system_error(errno, os_category(), "shutdown");
		}

		protocol_family const tcp = {
			pf_tcpx_construct_descriptor,
			pf_tcpx_destruct_descriptor,
			pf_tcpx_descriptor,
			pf_tcpx_new_listener,
			pf_tcpx_new_connection,
			pf_tcpx_set_nonblocking,
			pf_unimpl_local_address,
			pf_unimpl_remote_address,
			pf_unimpl_accept,
			pf_unimpl_send,
			pf_unimpl_recv,
			pf_unimpl_fin
		};

		protocol_family const tcp4 = {
			pf_tcpx_construct_descriptor,
			pf_tcpx_destruct_descriptor,
			pf_tcpx_descriptor,
			pf_tcp4_new_listener,
			pf_tcp4_new_connection,
			pf_tcpx_set_nonblocking,
			pf_tcp4_local_address,
			pf_tcp4_remote_address,
			pf_tcp4_accept,
			pf_tcpx_send,
			pf_tcpx_recv,
			pf_tcpx_fin
		};

		protocol_family const tcp6 = {
			pf_tcpx_construct_descriptor,
			pf_tcpx_destruct_descriptor,
			pf_tcpx_descriptor,
			pf_tcp6_new_listener,
			pf_tcp6_new_connection,
			pf_tcpx_set_nonblocking,
			pf_tcp6_local_address,
			pf_tcp6_remote_address,
			pf_tcp6_accept,
			pf_tcpx_send,
			pf_tcpx_recv,
			pf_tcpx_fin
		};

	}
}

