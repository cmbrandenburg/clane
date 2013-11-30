// vim: set noet:

/** \file */

#include "net_error.h"
#include "net_ip.h"
#include <arpa/inet.h>
#include <cctype>
#include <cstring>
#include <endian.h>
#include <functional>
#include <memory>
#include <netdb.h>
#include <sstream>
#include <stdexcept>

namespace clane {
	namespace net {

		protocol_family const *split_ipvx_host_port(char * const addr, char **ohost, char **oport);

		template <typename addr_T, int domain_N> class protocol_family_inet: public protocol_family {
		public:
			virtual int domain() const;
			virtual std::string local_addr(socket const &sock) const;
			virtual std::string remote_addr(socket const &sock) const;
			virtual accept_result accept(socket const &lis, std::string &oaddr) const;
			virtual void connect(socket const &conn, char const *addr, size_t addr_len) const;
			virtual recv_result recv(socket const &conn, void *buf, size_t cap, recv_options const *opts) const;
			virtual send_result send(socket const &conn, void const *buf, size_t cnt, send_options const *opts) const;
		private:
			static std::string translate_addr(addr_T const *sa);
			static std::string host_and_port(char const *host, char const *port);
			static addr_T resolve_addr(std::string const &addr);
			static addr_T resolve_addr(char const *addr, size_t addr_len);
		};

		// Wraps a struct addrinfo with RAII semantics, useful for getaddrinfo().
		class scoped_addrinfo {
			addrinfo *ai;
		public:
			~scoped_addrinfo();
			scoped_addrinfo(addrinfo *ai);
			scoped_addrinfo(scoped_addrinfo const &) = delete;
			scoped_addrinfo &operator=(scoped_addrinfo const &) = delete;
		};

		template <typename addr_T, int domain_N> accept_result protocol_family_inet<addr_T, domain_N>::accept(socket const &lis,
				std::string &oaddr) const {
			addr_T sa{};
			socklen_t sa_len{sizeof(sa)};
			auto astat = lis.accept(reinterpret_cast<sockaddr *>(&sa), &sa_len);
			if (astat.conn)
				oaddr = translate_addr(&sa);
			return astat;
		}

		template <typename addr_T, int domain_N> void protocol_family_inet<addr_T, domain_N>::connect(socket const &conn,
				char const *addr, size_t addr_len) const {
			addr_T sa = resolve_addr(addr, addr_len);
			conn.connect(reinterpret_cast<sockaddr const *>(&sa), sizeof(sa));
		}

		template <typename addr_T, int domain_N> int protocol_family_inet<addr_T, domain_N>::domain() const {
			return domain_N;
		}

		template <typename addr_T, int domain_N> std::string protocol_family_inet<addr_T, domain_N>::host_and_port(
				char const *host, char const *port) {
			std::ostringstream ss;
			ss << host << ":" << port;
			return ss.str();
		}

		template <> std::string protocol_family_inet<sockaddr_in6, PF_INET6>::host_and_port(
				char const *host, char const *port) {
			std::ostringstream ss;
			ss << "[" << host << "]:" << port;
			return ss.str();
		}

		template <typename addr_T, int domain_N> std::string protocol_family_inet<addr_T, domain_N>::local_addr(
				socket const &sock) const {
			addr_T sa{};
			socklen_t sa_len{sizeof(sa)};
			int status = getsockname(sock, reinterpret_cast<sockaddr *>(&sa), &sa_len);
			if (-1 == status) {
				std::ostringstream ss;
				ss << "error getting IP socket's local address: " << errno_to_string(errno);
				throw std::runtime_error(ss.str());
			}
			return translate_addr(&sa);
		}

		template <typename addr_T, int domain_N> recv_result protocol_family_inet<addr_T, domain_N>::recv(socket const &conn,
				void *buf, size_t cap, recv_options const *opts) const {
			static recv_options const default_opts{};
			if (!opts)
				opts = &default_opts;
			recv_result result{};
			int flags = opts->nonblocking ? MSG_DONTWAIT : 0;
			size_t sys_stat;
			while (static_cast<size_t>(-1) == (sys_stat = ::recv(conn, buf, cap, flags)) && EINTR == errno);
			if (static_cast<size_t>(-1) == sys_stat && (EAGAIN == errno || EWOULDBLOCK == errno))
				return result;
			if (static_cast<size_t>(-1) == sys_stat && (ECONNRESET == errno || ECONNABORTED == errno || ETIMEDOUT == errno)) {
				result.reset = true;
				return result;
			}
			if (0 == sys_stat) {
				result.shutdown = true;
				return result;
			}
			result.size = sys_stat;
			return result;
		}

		template <typename addr_T, int domain_N> std::string protocol_family_inet<addr_T, domain_N>::remote_addr(
				socket const &sock) const {
			addr_T sa{};
			socklen_t sa_len{sizeof(sa)};
			int status = getpeername(sock, reinterpret_cast<sockaddr *>(&sa), &sa_len);
			if (-1 == status) {
				std::ostringstream ss;
				ss << "error getting IP socket's remote address: " << errno_to_string(errno);
				throw std::runtime_error(ss.str());
			}
			return translate_addr(&sa);
		}

		template <typename addr_T, int domain_N> addr_T protocol_family_inet<addr_T, domain_N>::resolve_addr(
				char const *addr, size_t addr_len) {

			// parse address string:
			char *mut_addr = new char[addr_len+1];
			memcpy(mut_addr, addr, addr_len);
			mut_addr[addr_len] = 0;
			std::unique_ptr<char[]> mut_addr_owner(mut_addr);
			char *host, *port;
			split_ipvx_host_port(mut_addr, &host, &port);

			// resolve address:
			//
			// If the host string is empty then resolve to the loopback address.
			//
			// XXX: On Debian Squeeze, the AI_ALL flag is needed for name lookup not
			// to fail ("No address associated with hostname"). Why? The man page
			// makes it seem that AI_ALL isn't needed.
			//
			addrinfo addr_hints{}, *addr_res{};
			addr_hints.ai_flags = AI_V4MAPPED | AI_ALL;
			addr_hints.ai_family = domain_N;
			addr_hints.ai_socktype = SOCK_STREAM;
			addr_hints.ai_protocol = IPPROTO_TCP;
			int status = getaddrinfo(*host ? host : nullptr, port, &addr_hints, &addr_res);
			if (status) {
				std::ostringstream ss;
				ss << "error getting address info: " << gai_strerror(status);
				throw std::runtime_error(ss.str());
			}
			scoped_addrinfo addr_res_owner(addr_res);

			// Use the first resolved address. Ignore all other addresses.
			
			return *reinterpret_cast<addr_T const *>(addr_res->ai_addr);
		}

		template <typename addr_T, int domain_N> inline addr_T protocol_family_inet<addr_T, domain_N>::resolve_addr(
				std::string const &addr) {
			return resolve_addr(addr.c_str(), addr.size());
		}

		template <typename addr_T, int domain_N> send_result protocol_family_inet<addr_T, domain_N>::send(socket const &conn,
				void const *buf, size_t cnt, send_options const *opts) const {
			static send_options const default_opts{};
			if (!opts)
				opts = &default_opts;
			send_result result{};
			size_t status;
			int flags = MSG_NOSIGNAL;
			flags |= opts->nonblocking ? MSG_DONTWAIT : 0;
			while (static_cast<size_t>(-1) == (status = ::send(conn, buf, cnt, flags)) && EINTR == errno);
			if (static_cast<size_t>(-1) == status && (EAGAIN == errno || EWOULDBLOCK == errno)) {
				return result;
			}
			if (static_cast<size_t>(-1) == status &&
					(EPIPE == errno || ECONNRESET == errno || ECONNABORTED == errno || ETIMEDOUT == errno)) {
				result.reset = true;
				return result;
			}
			result.size = status;
			return result;
		}

		template <typename addr_T, int domain_N>
			std::string protocol_family_inet<addr_T, domain_N>::translate_addr(addr_T const *sa) {
			char host_buf[NI_MAXHOST];
			char port_buf[NI_MAXSERV];
			int status = getnameinfo(reinterpret_cast<sockaddr const *>(sa), sizeof(addr_T), host_buf, sizeof(host_buf),
					port_buf, sizeof(port_buf), NI_NUMERICHOST | NI_NUMERICSERV);
			if (status) {
				std::ostringstream ss;
				ss << "error translating socket address to name: " << errno_to_string(errno);
				throw std::runtime_error(ss.str());
			}
			return host_and_port(host_buf, port_buf);
		}

		// protocol family for IPv4:
		protocol_family_inet<sockaddr_in, PF_INET> pf_ipv4;

		// protocol family for IPv6:
		protocol_family_inet<sockaddr_in6, PF_INET6> pf_ipv6;

		socket dial_tcp(char const *addr, size_t addr_len, tcp_dial_opts const *opts) {
			static tcp_dial_opts default_dial_opts;
			if (!opts)
				opts = &default_dial_opts;
			socket sock(&pf_ipv6, SOCK_STREAM, IPPROTO_TCP);
			sock.connect(addr, addr_len);
			return sock;
		}

		socket listen_tcp(char const *addr, size_t addr_len, int backlog, tcp_listen_opts const *opts) {
			static tcp_listen_opts default_listen_opts;
			if (!opts)
				opts = &default_listen_opts;

			// parse address string:
			char *mut_addr = new char[addr_len+1];
			memcpy(mut_addr, addr, addr_len);
			mut_addr[addr_len] = 0;
			std::unique_ptr<char[]> mut_addr_owner(mut_addr);
			char *host, *port;
			protocol_family const *pf = split_ipvx_host_port(mut_addr, &host, &port);

			// resolve bind address:
			//
			// If the host string is empty then bind to all local addresses. If the
			// port string is empty then bind to an arbitrary port.
			//
			// XXX: On Debian Squeeze, the AI_ALL flag is needed for name lookup not
			// to fail ("No address associated with hostname"). Why? The man page
			// makes it seem that AI_ALL isn't needed.
			//
			addrinfo addr_hints{}, *addr_res{};
			addr_hints.ai_flags = AI_PASSIVE | AI_V4MAPPED | AI_ALL;
			addr_hints.ai_family = pf->domain();
			addr_hints.ai_socktype = SOCK_STREAM;
			addr_hints.ai_protocol = IPPROTO_TCP;
			int status = getaddrinfo(*host ? host : nullptr, *port ? port : nullptr, &addr_hints, &addr_res);
			if (status) {
				std::ostringstream ss;
				ss << "error getting address info: " << gai_strerror(status);
				throw std::runtime_error(ss.str());
			}
			scoped_addrinfo addr_res_owner(addr_res);

			// create socket:
			socket sock(pf, SOCK_STREAM, IPPROTO_TCP);
			if (opts->reuse_addr)
				sock.set_reuseaddr();
			sock.bind(addr_res->ai_addr, addr_res->ai_addrlen);
			sock.listen(backlog);
			return sock;
		}

		protocol_family const *split_ipvx_host_port(char * const addr, char **ohost, char **oport) {

			// Algorithm:
			// 1. If the notation begins with a bracket ('[') then the host is an IPv6
			// address.
			// 2. Else if the notation can be converted as an IPv4 dotted decimal
			// address then the host is an IPv4 address.
			// 3. Else the address is an IPv6 address.
			//
			// Basically, an address is IPv6 unless it's stated as a dotted decimal
			// IPv4 address.

			auto error = [&addr]() {
				std::ostringstream ss;
				ss << "invalid address \"" << addr << "\"";
				throw std::invalid_argument(ss.str());
			};
			// sanity check: disallow whitespace
			for (char const *p = addr; *p; ++p)
				if (isspace(*p))
					error();
			char *host, *end_host, *sep;
			protocol_family const *pf;
			host = addr;
			if ('[' == *host) {
				// bracketed -> IPv6:
				pf = &pf_ipv6;
				++host;
				end_host = strchr(host, ']');
				if (!end_host)
					error(); // missing closing bracket
				sep = end_host + 1;
				if (':' != *sep)
					error(); // missing separator colon between host and port
			} else {
				sep = strchr(host, ':');
				if (!sep || ':' != *sep)
					error(); // missing separator colon between host and port
				*sep = 0; // mutation -> can't fail after this
				end_host = sep;
				in_addr dummy_ipv4_addr;
				int status = inet_pton(AF_INET, host, &dummy_ipv4_addr);
				if (-1 == status) {
					std::ostringstream ss;
					ss << "error converting from IPv4 dotted decimal notation: " << errno_to_string(errno);
					throw std::runtime_error(ss.str());
				}
				if (1 == status) {
					// dotted decimal -> IPv4:
					pf = &pf_ipv4;
				} else {
					// hostname -> IPv6
					pf = &pf_ipv6;
				}
			}
			*ohost = host;
			*end_host = 0;
			*oport = sep + 1;
			return pf;
		}

		scoped_addrinfo::~scoped_addrinfo() {
			freeaddrinfo(ai);
		}

		scoped_addrinfo::scoped_addrinfo(addrinfo *ai) : ai(ai) {
		}

	}
}

