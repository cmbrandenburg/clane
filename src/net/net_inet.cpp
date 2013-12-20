// vim: set noet:

/** \file */

#include "net_error.h"
#include "net_fd.h"
#include "net_inet.h"
#include "net_socket.h"
#include <arpa/inet.h>
#include <cstring>
#include <iomanip>
#include <netdb.h>
#include <netinet/ip.h>
#include <poll.h>
#include <sstream>
#include <stdexcept>
#include <unistd.h>

namespace clane {
	namespace net {

		class scoped_addrinfo {
			addrinfo *ai;
		public:
			~scoped_addrinfo() { if (ai) { freeaddrinfo(ai); }}
			scoped_addrinfo(addrinfo *ai): ai(ai) {}
			scoped_addrinfo(scoped_addrinfo const &) = delete;
			scoped_addrinfo(scoped_addrinfo &&) = default;
			scoped_addrinfo &operator=(scoped_addrinfo const &) = delete;
			scoped_addrinfo &operator=(scoped_addrinfo &&) = default;
			addrinfo *operator->() { return ai; }
			addrinfo const *operator->() const { return ai; }
		};

		char const *pf_tcp_name() { return "tcp"; }
		void pf_tcp_construct_instance(protocol_family::instance &pi);
		void pf_tcp_destruct_instance(protocol_family::instance &pi);
		connect_result pf_tcp_connect(protocol_family::instance &pi, std::string &addr);
		send_result pf_tcp_send(protocol_family::instance &pi, void const *p, size_t n, int flags);
		recv_result pf_tcp_recv(protocol_family::instance &pi, void *p, size_t n, int flags);
		protocol_family const *pf_tcp_listen(protocol_family::instance &pi, std::string &addr, int backlog);
		int pf_tcp_domain() { return AF_UNSPEC; }

		static protocol_family const pf_tcp = {
			pf_tcp_name,
			pf_tcp_construct_instance,
			pf_tcp_destruct_instance,
			pf_unsupported_local_address,
			pf_unsupported_remote_address,
			pf_tcp_connect,
			pf_unsupported_send,
			pf_unsupported_recv,
			pf_tcp_listen,
			pf_unsupported_accept,
			pf_tcp_domain
		};

		protocol_family const *tcp = &pf_tcp;

		char const *pf_tcp4_name() { return "tcp4"; }
		std::string pf_tcp4_local_address(protocol_family::instance &pi);
		std::string pf_tcp4_remote_address(protocol_family::instance &pi);
		connect_result pf_tcp4_connect(protocol_family::instance &pi, std::string &addr);
		protocol_family const *pf_tcp4_listen(protocol_family::instance &pi, std::string &addr, int backlog);
		accept_result pf_tcp4_accept(protocol_family::instance &pi, std::string *oaddr, int flags);
		int pf_tcp4_domain() { return AF_INET; }

		static protocol_family const pf_tcp4 = {
			pf_tcp4_name,
			pf_tcp_construct_instance,
			pf_tcp_destruct_instance,
			pf_tcp4_local_address,
			pf_tcp4_remote_address,
			pf_tcp4_connect,
			pf_tcp_send,
			pf_tcp_recv,
			pf_tcp4_listen,
			pf_tcp4_accept,
			pf_tcp4_domain
		};

		protocol_family const *tcp4 = &pf_tcp4;

		char const *pf_tcp6_name() { return "tcp6"; }
		std::string pf_tcp6_local_address(protocol_family::instance &pi);
		std::string pf_tcp6_remote_address(protocol_family::instance &pi);
		connect_result pf_tcp6_connect(protocol_family::instance &pi, std::string &addr);
		protocol_family const *pf_tcp6_listen(protocol_family::instance &pi, std::string &addr, int backlog);
		accept_result pf_tcp6_accept(protocol_family::instance &pi, std::string *oaddr, int flags);
		int pf_tcp6_domain() { return AF_INET6; }

		static protocol_family const pf_tcp6 = {
			pf_tcp6_name,
			pf_tcp_construct_instance,
			pf_tcp_destruct_instance,
			pf_tcp6_local_address,
			pf_tcp6_remote_address,
			pf_tcp6_connect,
			pf_tcp_send,
			pf_tcp_recv,
			pf_tcp6_listen,
			pf_tcp6_accept,
			pf_tcp6_domain
		};

		protocol_family const *tcp6 = &pf_tcp6;

		void pf_tcp_construct_instance(protocol_family::instance &pi) {
			pi.n = -1;
		}

		void pf_tcp_destruct_instance(protocol_family::instance &pi) {
			if (-1 != pi.n) {
				close_fd(pi.n);
				pi.n = -1;
			}
		}

		scoped_addrinfo resolve_address_tcp(protocol_family const *pf, std::string &addr) {
			int domain = pf->domain();
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
			hints.ai_flags = AI_ADDRCONFIG;
			hints.ai_family = domain;
			hints.ai_socktype = SOCK_STREAM;
			hints.ai_protocol = 0;
			addrinfo *results{};
			// FIXME: Does getaddrinfo ever interrupt due to EINTR?
			int stat = ::getaddrinfo(addr.data() + host, addr.data() + port, &hints, &results);
			if (stat) {
				std::ostringstream ess;
				addr[sep] = saved_sep; // restore address
				ess << "address translation error: " <<
				 	((EAI_SYSTEM == stat) ? errno_to_string(errno) : std::string(gai_strerror(stat)));
				throw std::runtime_error(ess.str());
			}
			return results;
		}

		protocol_family const *protocol_family_tcp_by_domain(int domain) {
			switch (domain) {
				case AF_INET:
					return &pf_tcp4;
				case AF_INET6:
					return &pf_tcp6;
				default: {
					std::ostringstream ess;
					ess << "invalid TCP domain " << domain;
					throw std::logic_error(ess.str());
				}
			}
		}

		connect_result tcpx_connect(protocol_family const *pf, protocol_family::instance &pi, std::string &addr) {
			connect_result result{};
			auto lookup = resolve_address_tcp(pf, addr);
			pi.n = sys_socket(lookup->ai_family, SOCK_STREAM, 0);
			int stat = TEMP_FAILURE_RETRY(::connect(pi.n, lookup->ai_addr, lookup->ai_addrlen));
			if (-1 == stat && ECONNREFUSED == errno) {
				result.stat = status::conn_refused;
				return result;
			}
			if (-1 == stat && ENETUNREACH == errno) {
				result.stat = status::net_unreachable;
				return result;
			}
			if (-1 == stat && ETIMEDOUT == errno) {
				result.stat = status::timed_out;
				return result;
			}
			result.pf = protocol_family_tcp_by_domain(lookup->ai_family);
			return result;
		}

		connect_result pf_tcp_connect(protocol_family::instance &pi, std::string &addr) {
			return tcpx_connect(tcp, pi, addr);
		}

		connect_result pf_tcp4_connect(protocol_family::instance &pi, std::string &addr) {
			return tcpx_connect(tcp4, pi, addr);
		}

		connect_result pf_tcp6_connect(protocol_family::instance &pi, std::string &addr) {
			return tcpx_connect(tcp6, pi, addr);
		}

		protocol_family const *tcpx_listen(protocol_family const *pf, protocol_family::instance &pi, std::string &addr, int backlog) {
			auto lookup = resolve_address_tcp(pf, addr);
			pi.n = sys_socket(lookup->ai_family, SOCK_STREAM, 0);
			set_nonblocking(pi.n);
			sys_setsockopt(pi.n, SOL_SOCKET, SO_REUSEADDR, 1);
			sys_bind(pi.n, lookup->ai_addr, lookup->ai_addrlen);
			sys_listen(pi.n, -1 == backlog ? 256 : backlog);
			return protocol_family_tcp_by_domain(lookup->ai_family);
		}

		protocol_family const *pf_tcp_listen(protocol_family::instance &pi, std::string &addr, int backlog) {
			return tcpx_listen(tcp, pi, addr, backlog);
		}

		protocol_family const *pf_tcp4_listen(protocol_family::instance &pi, std::string &addr, int backlog) {
			return tcpx_listen(tcp4, pi, addr, backlog);
		}

		protocol_family const *pf_tcp6_listen(protocol_family::instance &pi, std::string &addr, int backlog) {
			return tcpx_listen(tcp6, pi, addr, backlog);
		}

		send_result pf_tcp_send(protocol_family::instance &pi, void const *p, size_t n, int flags) {
			send_result result{};
			int sys_flags = MSG_NOSIGNAL;
			if (flags & op_nonblock)
				sys_flags |= MSG_DONTWAIT;
			char const *ppos = reinterpret_cast<char const *>(p);
			do {
				ssize_t stat = TEMP_FAILURE_RETRY(::send(pi.n, ppos, n-result.size, sys_flags));
				if (-1 == stat && flags & op_nonblock && (EAGAIN == errno || EWOULDBLOCK == errno)) {
					if (!result.size)
						result.stat = status::would_block;
					return result;
				}
				if (-1 == stat && (EPIPE == errno || ECONNRESET == errno)) {
					result.stat = status::reset;
					return result;
				}
				if (-1 == stat && ETIMEDOUT == errno) {
					result.stat = status::timed_out;
					return result;
				}
				if (-1 == stat) {
					std::ostringstream ess;
					ess << "socket send error (sock=" << pi.n << ", flags=" << std::showbase << std::hex << flags << "): " <<
						errno_to_string(errno);
					throw std::runtime_error(ess.str());
				}
				result.size += stat;
				ppos += stat;
			} while (!(flags & op_nonblock) && flags & op_all && result.size < n);
			if (result.size == n && flags & op_fin)
				sys_shutdown(pi.n, SHUT_WR);
			return result;
		}

		recv_result pf_tcp_recv(protocol_family::instance &pi, void *p, size_t n, int flags) {
			recv_result result{};
			int sys_flags = 0;
			if (flags & op_nonblock)
				sys_flags |= MSG_DONTWAIT;
			else if (flags & op_all)
				sys_flags |= MSG_WAITALL;
			char *ppos = reinterpret_cast<char *>(p);
			do {
				ssize_t stat = TEMP_FAILURE_RETRY(::recv(pi.n, ppos, n-result.size, sys_flags));
				if (-1 == stat && flags & op_nonblock && (EAGAIN == errno || EWOULDBLOCK == errno)) {
					result.stat = status::would_block;
					return result;
				}
				if (-1 == stat && ECONNRESET == errno) {
					result.stat = status::reset;
					return result;
				}
				if (-1 == stat && ETIMEDOUT == errno) {
					result.stat = status::timed_out;
					return result;
				}
				if (-1 == stat) {
					std::ostringstream ess;
					ess << "socket receive error (sock=" << pi.n << ", flags=" << std::showbase << std::hex << flags << "): " <<
						errno_to_string(errno);
					throw std::runtime_error(ess.str());
				}
				if (0 == stat)
					break;
				result.size += stat;
				ppos += stat;
			} while (flags & op_all && result.size < n);
			return result;
		}

		std::string tcp4_address_to_string(sockaddr_in const &sa) {
			std::ostringstream ss;
			char buf[INET_ADDRSTRLEN];
			char const *addr = inet_ntop(AF_INET, &sa.sin_addr, buf, sizeof(buf));
			if (!addr) {
				std::ostringstream ess;
				ess << "IP address-to-string conversion error: " << errno_to_string(errno);
				throw std::runtime_error(ess.str());
			}
			ss << addr << ':' << be16toh(sa.sin_port);
			return ss.str();
		}

		std::string tcp6_address_to_string(sockaddr_in6 const &sa) {
			std::ostringstream ss;
			char buf[INET6_ADDRSTRLEN];
			char const *addr = inet_ntop(AF_INET6, &sa.sin6_addr, buf, sizeof(buf));
			if (!addr) {
				std::ostringstream ess;
				ess << "IPv6 address-to-string conversion error: " << errno_to_string(errno);
				throw std::runtime_error(ess.str());
			}
			ss << '[' << addr << "]:" << be16toh(sa.sin6_port);
			return ss.str();
		}

		std::string pf_tcp4_local_address(protocol_family::instance &pi) {
			sockaddr_in sa;
			sys_getsockname(pi.n, reinterpret_cast<sockaddr *>(&sa), sizeof(sa));
			return tcp4_address_to_string(sa);
		}

		std::string pf_tcp4_remote_address(protocol_family::instance &pi) {
			sockaddr_in sa;
			sys_getpeername(pi.n, reinterpret_cast<sockaddr *>(&sa), sizeof(sa));
			return tcp4_address_to_string(sa);
		}

		std::string pf_tcp6_local_address(protocol_family::instance &pi) {
			sockaddr_in6 sa;
			sys_getsockname(pi.n, reinterpret_cast<sockaddr *>(&sa), sizeof(sa));
			return tcp6_address_to_string(sa);
		}

		std::string pf_tcp6_remote_address(protocol_family::instance &pi) {
			sockaddr_in6 sa;
			sys_getpeername(pi.n, reinterpret_cast<sockaddr *>(&sa), sizeof(sa));
			return tcp6_address_to_string(sa);
		}

		accept_result pf_tcp_accept(protocol_family const *pf, protocol_family::instance &pi,
			   	sockaddr *addr, socklen_t addr_len, int flags) {
			accept_result result{};
			while (true) {
				socklen_t sa_len = addr_len;
				file_descriptor fd = TEMP_FAILURE_RETRY(::accept(pi.n, addr, &sa_len));
				if (-1 == fd && (EAGAIN == errno || EWOULDBLOCK == errno)) {
					if (flags & op_nonblock) {
						result.stat = status::would_block;
						return result;
					}
					pollfd pent;
					pent.fd = pi.n;
					pent.events = POLLIN;
					if (-1 == ::poll(&pent, 1, -1)) {
						std::ostringstream ess;
						ess << "poll error: " << errno_to_string(errno);
						throw std::runtime_error(ess.str());
					}
					continue; // discard poll result, try to accept again
				}
				if (-1 == fd && ECONNABORTED == errno) {
					result.stat = status::aborted;
					return result;
				}
				if (-1 == fd && (EMFILE == errno || ENFILE == errno || ENOBUFS == errno || ENOMEM == errno)) {
					result.stat = status::no_resource;
					return result;
				}
				if (-1 == fd) {
					std::ostringstream ess;
					ess << "socket accept error (sock=" << pi.n << ", flags=" << std::showbase << std::hex << flags << "): " <<
						errno_to_string(errno);
					throw std::runtime_error(ess.str());
				}
				protocol_family::instance sock_inst;
				sock_inst.n = fd.release();
				result.sock = socket(pf, std::move(sock_inst));
				return result;
			}
		}

		accept_result pf_tcp4_accept(protocol_family::instance &pi, std::string *oaddr, int flags) {
			sockaddr_in sa;
			accept_result result = pf_tcp_accept(&pf_tcp4, pi, reinterpret_cast<sockaddr *>(&sa), sizeof(sa), flags);
			if (status::ok == result.stat && oaddr)
				*oaddr = tcp4_address_to_string(sa);
			return result;
		}

		accept_result pf_tcp6_accept(protocol_family::instance &pi, std::string *oaddr, int flags) {
			sockaddr_in6 sa;
			accept_result result = pf_tcp_accept(&pf_tcp6, pi, reinterpret_cast<sockaddr *>(&sa), sizeof(sa), flags);
			if (status::ok == result.stat && oaddr)
				*oaddr = tcp6_address_to_string(sa);
			return result;
		}
	}
}

