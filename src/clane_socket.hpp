// vim: set noet:

#ifndef CLANE__SOCKET_HPP
#define CLANE__SOCKET_HPP

#include "clane_common.hpp"
#include "clane_posix.hpp"
#include <sys/socket.h>

namespace clane {
	namespace net {

		enum class status {
			ok,
			would_block,
			in_progress,
			timed_out,
			conn_refused,
			net_unreachable,
			reset,
			aborted,
			no_resource,
			permission
		};

		// low-level socket functions:
		posix::file_descriptor sys_socket(int domain, int type, int protocol);
		void sys_setsockopt(int sock_fd, int level, int optname, int val);
		void sys_bind(int sockfd, sockaddr const *addr, socklen_t addr_len);
		void sys_listen(int sockfd, int backlog);
		void sys_getsockname(int sockfd, sockaddr *addr, socklen_t addr_len);
		void sys_getpeername(int sockfd, sockaddr *addr, socklen_t addr_len);
		status sys_connect(int sockfd, sockaddr const *addr, socklen_t addr_len);

		union socket_descriptor {
			int n;
			void *p;
		};

		class socket;
		struct connect_result;

		struct protocol_family {
			void (*construct_descriptor)(socket_descriptor &sd);
			void (*destruct_descriptor)(socket_descriptor &sd);
			socket (*new_listener)(std::string &addr, int backlog);
			connect_result (*new_connection)(std::string &addr);
			std::string (*local_address)(socket_descriptor &sd);
			std::string (*remote_address)(socket_descriptor &sd);
		};

		// default protocol family method implementations:
		void pf_unimpl_construct_descriptor(socket_descriptor &);
		void pf_unimpl_destruct_destriptor(socket_descriptor &);
		socket pf_unimpl_new_listener(std::string &, int);
		connect_result pf_unimpl_new_connection(std::string &);
		std::string pf_unimpl_local_address(socket_descriptor &);
		std::string pf_unimpl_remote_address(socket_descriptor &);

		class socket {
			protocol_family const *pf;
			socket_descriptor sd;
		public:
			~socket() { if (pf) { pf->destruct_descriptor(sd); }}
			socket() noexcept: pf{} {}
			socket(protocol_family const *pf, posix::file_descriptor &&fd): pf{pf} { sd.n = fd.release(); }
			socket(socket const &) = delete;
			socket(socket &&that) noexcept: pf{} { swap(that); }
			socket &operator=(socket const &) = delete;
			socket &operator=(socket &&that) noexcept;
			void swap(socket &that) noexcept;
			std::string local_address() { return pf->local_address(sd); }
			std::string remote_address() { return pf->remote_address(sd); }
		};

		inline socket &socket::operator=(socket &&that) noexcept {
			swap(that);
			return *this;
		}

		inline void socket::swap(socket &that) noexcept {
			std::swap(pf, that.pf);
			std::swap(sd, that.sd);
		}

		inline socket listen(protocol_family const *pf, std::string addr, int backlog = -1) {
			return pf->new_listener(addr, backlog);
		}

		inline socket listen(protocol_family const *pf, char const *addr, int backlog = -1) {
			return listen(pf, std::string(addr), backlog);
		}

		struct connect_result {
			status stat;
			socket sock;
		};

		inline connect_result connect(protocol_family const *pf, std::string addr) {
			return pf->new_connection(addr);
		}

		inline connect_result connect(protocol_family const *pf, char const *addr) {
			return connect(pf, std::string(addr));
		}

#if 0
		struct connect_result {
			status stat;
			protocol_family const *pf;
		};

		struct accept_result;

		struct xfer_result {
			status stat;
			size_t size;
		};

		char const *what(status stat);

		template <class CharT, class Traits>
	 	std::basic_ostream<CharT, Traits> &operator<<(std::basic_ostream<CharT, Traits> &os, status stat) {
			return os << what(stat);
		}

		struct accept_result {
			status stat;
			socket sock;
		};


		struct protocol_family {
			char const *(*name)();
			std::string (*local_address)(instance &pi);
			std::string (*remote_address)(instance &pi);
			connect_result (*connect)(instance &pi, std::string &addr);
			send_result (*send)(instance &pi, void const *p, size_t n, int flags);
			recv_result (*recv)(instance &pi, void *p, size_t n, int flags);
			protocol_family const *(*listen)(instance &pi, std::string &addr, int backlog);
			accept_result (*accept)(instance &pi, std::string *oaddr, int flags);
			int (*domain)();
		};

		class socket {
			friend void swap(socket &, socket &) noexcept;
		private:
			protocol_family const *pf;
			protocol_family::instance pi;
		public:
			~socket();
#if 0
			socket() noexcept: pf{}, pi{} {}
			socket(protocol_family const *pf, char const *addr, int backlog): socket(pf, std::string(addr), backlog) {}
			socket(protocol_family const *pf, std::string addr, int backlog);
			socket(protocol_family const *pf, char const *addr): socket(pf, std::string(addr)) {}
			socket(protocol_family const *pf, std::string addr);
			socket(protocol_family const *pf, protocol_family::instance &&that_inst) noexcept;
			socket(socket const &) = delete;
			socket(socket &&that) noexcept: pf{}, pi{} { swap(*this, that); }
			socket &operator=(socket const &) = delete;
			socket &operator=(socket &&that) noexcept { swap(*this, that); return *this; }
			void listen(protocol_family const *pf, char const *addr, int backlog = -1) { listen(pf, std::string(addr)); }
			void listen(protocol_family const *pf, std::string addr, int backlog = -1);
			status connect(protocol_family const *pf, char const *addr) { return connect(pf, std::string(addr)); }
			status connect(protocol_family const *pf, std::string addr);
			protocol_family const *protocol() const { return pf; }
			std::string local_address() { return pf->local_address(pi); }
			std::string remote_address() { return pf->remote_address(pi); }
			send_result send(void const *p, size_t n, int flags = 0) { return pf->send(pi, p, n, flags); }
			recv_result recv(void *p, size_t n, int flags = 0) { return pf->recv(pi, p, n, flags); }
			accept_result accept(int flags = 0);
			accept_result accept(std::string &oaddr, int flags = 0);
			int fd() const { return pi.n; }
		private:
			status connect_(protocol_family const *pf, std::string &addr);
#endif
		};
#endif

	}
}

#endif // #ifndef CLANE__SOCKET_HPP
