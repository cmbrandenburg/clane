// vim: set noet:

#include "clane_inet.hpp"
#include "clane_socket.hpp"
#include <arpa/inet.h>
#include <netdb.h>
#include <sstream>
#include <stdexcept>
#include <unistd.h>

namespace clane {
	namespace net {

		protocol_family const *tcp_protocol_family_by_domain(int domain) {
			switch (domain) {
				case AF_INET: return &tcp4;
				case AF_INET6: return &tcp6;
				default: {
					std::ostringstream ess;
					ess << "invalid TCP domain " << domain;
					throw std::logic_error(ess.str());
				}
			}
		}

		std::string tcp4_address_to_string(sockaddr_in const &sa) {
			std::ostringstream ss;
			char buf[INET_ADDRSTRLEN];
			char const *addr = inet_ntop(AF_INET, &sa.sin_addr, buf, sizeof(buf));
			if (!addr) {
				std::ostringstream ess;
				ess << "convert IP address to string: " << posix::errno_to_string(errno);
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
				ess << "convert IPv6 address to string: " << posix::errno_to_string(errno);
				throw std::runtime_error(ess.str());
			}
			ss << '[' << addr << "]:" << be16toh(sa.sin6_port);
			return ss.str();
		}

		class addrinfo_ptr {
			addrinfo *ai;
		public:
			~addrinfo_ptr() { if (ai) { freeaddrinfo(ai); }}
			addrinfo_ptr(addrinfo *ai): ai(ai) {}
			addrinfo_ptr(addrinfo_ptr const &) = delete;
			addrinfo_ptr(addrinfo_ptr &&) = default;
			addrinfo_ptr &operator=(addrinfo_ptr const &) = delete;
			addrinfo_ptr &operator=(addrinfo_ptr &&) = default;
			addrinfo *operator->() { return ai; }
			addrinfo const *operator->() const { return ai; }
		};

		addrinfo_ptr resolve_inet_address(int domain, int type, bool passive, std::string &addr) {
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
				std::ostringstream ess;
				addr[sep] = saved_sep; // restore address
				ess << "address translation error: " <<
				 	((EAI_SYSTEM == stat) ? posix::errno_to_string(errno) : std::string(gai_strerror(stat)));
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

		connect_result pf_tcp_new_connection(int domain, std::string &addr) {
			auto lookup = resolve_inet_address(domain, SOCK_STREAM, false, addr);
			auto sock_fd = sys_socket(lookup->ai_family, SOCK_STREAM, 0);
			connect_result res{};
			res.stat = sys_connect(sock_fd, lookup->ai_addr, lookup->ai_addrlen);
			if (status::ok == res.stat || status::in_progress == res.stat) {
				set_nonblocking(sock_fd);
				res.sock = socket(tcp_protocol_family_by_domain(lookup->ai_family), std::move(sock_fd));
			}
			return res;
		}

		connect_result pf_tcpx_new_connection(std::string &addr) { return pf_tcp_new_connection(AF_UNSPEC, addr); }
		connect_result pf_tcp4_new_connection(std::string &addr) { return pf_tcp_new_connection(AF_INET, addr); }
		connect_result pf_tcp6_new_connection(std::string &addr) { return pf_tcp_new_connection(AF_INET6, addr); }

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

		accept_result pf_tcp4_accept(socket_descriptor &sd, std::string *addr_o) {
			sockaddr_in sa;
			auto sys_res = sys_accept(sd.n, reinterpret_cast<sockaddr *>(addr_o ? &sa : nullptr), sizeof(sa));
			accept_result res{};
			res.stat = sys_res.first;
			if (status::ok == res.stat) {
				res.sock = socket(&tcp4, sys_res.second.release());
				if (addr_o)
					*addr_o = tcp4_address_to_string(sa);
			}
			return res;
		}

		accept_result pf_tcp6_accept(socket_descriptor &sd, std::string *addr_o) {
			sockaddr_in6 sa;
			auto sys_res = sys_accept(sd.n, reinterpret_cast<sockaddr *>(addr_o ? &sa : nullptr), sizeof(sa));
			accept_result res{};
			res.stat = sys_res.first;
			if (status::ok == res.stat) {
				res.sock = socket(&tcp6, sys_res.second.release());
				if (addr_o)
					*addr_o = tcp6_address_to_string(sa);
			}
			return res;
		}

		xfer_result pf_tcpx_send(socket_descriptor &sd, void const *p, size_t n) {
			xfer_result res{};
			ssize_t stat = TEMP_FAILURE_RETRY(::send(sd.n, p, n, MSG_NOSIGNAL));
			if (-1 == stat) {
				switch (errno) {
					case EACCES: res.stat = status::permission; return res;
					case EAGAIN:
#if EAGAIN != EWOULDBLOCK
					case EWOULDBLOCK:
#endif
						res.stat = status::would_block; return res;
					case ECONNRESET: case EPIPE: res.stat = status::reset; return res;
					case ENOBUFS: case ENOMEM: res.stat = status::no_resource; return res;
					case ETIMEDOUT: res.stat = status::timed_out; return res;
				}
				if (-1 == stat) {
					std::ostringstream ess;
					ess << "socket send (sockfd=" << sd.n << "): " << posix::errno_to_string(errno);
					throw std::runtime_error(ess.str());
				}
			}
			res.size = stat;
			return res;
		}

		xfer_result pf_tcpx_recv(socket_descriptor &sd, void *p, size_t n) {
			xfer_result res{};
			ssize_t stat = TEMP_FAILURE_RETRY(::recv(sd.n, p, n, 0));
			if (-1 == stat) {
				switch (errno) {
					case EACCES: res.stat = status::permission; return res;
					case EAGAIN:
#if EAGAIN != EWOULDBLOCK
					case EWOULDBLOCK:
#endif
						res.stat = status::would_block; return res;
					case ECONNREFUSED: res.stat = status::conn_refused; return res;
					case ECONNRESET: res.stat = status::reset; return res;
					case ENOMEM: res.stat = status::no_resource; return res;
					case ETIMEDOUT: res.stat = status::timed_out; return res;
				}
				if (-1 == stat) {
					std::ostringstream ess;
					ess << "socket receive (sockfd=" << sd.n << "): " << posix::errno_to_string(errno);
					throw std::runtime_error(ess.str());
				}
			}
			res.size = stat;
			return res;
		}

		void pf_tcpx_fin(socket_descriptor &sd) {
			int stat = ::shutdown(sd.n, SHUT_WR);
			if (-1 == stat) {
				std::ostringstream ess;
				ess << "socket shutdown: (sockfd=" << sd.n << "): " << posix::errno_to_string(errno);
				throw std::runtime_error(ess.str());
			}
		}

		protocol_family const tcp = {
			pf_tcpx_construct_descriptor,
			pf_tcpx_destruct_descriptor,
			pf_tcpx_descriptor,
			pf_tcpx_new_listener,
			pf_tcpx_new_connection,
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
			pf_tcp6_local_address,
			pf_tcp6_remote_address,
			pf_tcp6_accept,
			pf_tcpx_send,
			pf_tcpx_recv,
			pf_tcpx_fin
		};
	}
}

