// vim: set noet:

#ifndef CLANE_NET_SOCKET_H
#define CLANE_NET_SOCKET_H

/** @file
 *
 * @brief Socket type and system socket routines */

#include "net_common.h"
#include <ostream>
#include <sys/socket.h>

namespace clane {
	namespace net {

		class protocol_family;

		int sys_socket(int domain, int type, int protocol);
		void sys_bind(int sock, sockaddr const *addr, socklen_t addr_len);
		void sys_listen(int sock, int backlog);
		void sys_getsockname(int sock, sockaddr *addr, socklen_t addr_len);
		void sys_getpeername(int sock, sockaddr *addr, socklen_t addr_len);
		void sys_shutdown(int sock, int how);
		int sys_getsockopt(int sock, int level, int optname);
		void sys_setsockopt(int sock, int level, int optname, int val);

		enum class status {
			ok,
			would_block,
			in_progress,
			timed_out,
			conn_refused,
			net_unreachable,
			reset,
			aborted,
			no_resource
		};

		char const *what(status stat);

		template <class CharT, class Traits>
	 	std::basic_ostream<CharT, Traits> &operator<<(std::basic_ostream<CharT, Traits> &os, status stat) {
			return os << what(stat);
		}

		// Operation flags
		enum {
			op_nonblock = 1<<0,
			op_fin = 1<<1,
			op_all = 1<<2
		};

		struct protocol_family;

		struct connect_result {
			status stat;
			protocol_family const *pf;
		};

		struct accept_result;

		struct send_result {
			status stat;
			size_t size;
		};

		struct recv_result {
			status stat;
			size_t size;
		};

		struct protocol_family {
			union instance {
				int n;
				void *ptr;
			};
			char const *(*name)();
			void (*construct_instance)(instance &pi);
			void (*destruct_instance)(instance &pi);
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
			~socket() noexcept;
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
		};

		inline socket::socket(protocol_family const *pf, protocol_family::instance &&that_inst) noexcept: pf(pf), pi{} {
			std::swap(pi, that_inst);
		}

		inline void swap(socket &a, socket &b) noexcept {
			std::swap(a.pf, b.pf);
			std::swap(a.pi, b.pi);
		}

		struct accept_result {
			status stat;
			socket sock;
		};

		inline accept_result socket::accept(int flags) {
			return pf->accept(pi, nullptr, flags);
	 	}

		inline accept_result socket::accept(std::string &oaddr, int flags) {
		 	return pf->accept(pi, &oaddr, flags);
	 	}

		// stand-ins for unsupported operations:
		void pf_unsupported_construct_instance(protocol_family::instance &);
		void pf_unsupported_destruct_instance(protocol_family::instance &);
		std::string pf_unsupported_local_address(protocol_family::instance &);
		std::string pf_unsupported_remote_address(protocol_family::instance &);
		connect_result pf_unsupported_connect(protocol_family::instance &, std::string &);
		send_result pf_unsupported_send(protocol_family::instance &, void const *, size_t , int);
		recv_result pf_unsupported_recv(protocol_family::instance &, void *, size_t , int);
		protocol_family const *pf_unsupported_listen(protocol_family::instance &, std::string &, int);
		accept_result pf_unsupported_accept(protocol_family::instance &, std::string *, int);
		int pf_unsupported_domain();

	}
}

#endif // #ifndef CLANE_NET_SOCKET_H
