// vim: set noet:

#ifndef CLANE__NET_SOCKET_HPP
#define CLANE__NET_SOCKET_HPP

#include "net_common.hpp"
#include "net_posix.hpp"
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
		std::pair<status, posix::file_descriptor> sys_accept(int sockfd, sockaddr *addr, socklen_t addr_len);

		union socket_descriptor {
			int n;
			void *p;
		};

		class socket;
		struct connect_result;
		struct accept_result;

		struct xfer_result {
			status stat;
			size_t size;
		};

		struct protocol_family {
			void (*construct_descriptor)(socket_descriptor &sd);
			void (*destruct_descriptor)(socket_descriptor &sd);
			int (*descriptor)(socket_descriptor const &sd);
			socket (*new_listener)(std::string &addr, int backlog);
			connect_result (*new_connection)(std::string &addr);
			std::string (*local_address)(socket_descriptor &sd);
			std::string (*remote_address)(socket_descriptor &sd);
			accept_result (*accept)(socket_descriptor &sd, std::string *addr_o);
			xfer_result (*send)(socket_descriptor &sd, void const *p, size_t n);
			xfer_result (*send_all)(socket_descriptor &sd, void const *p, size_t n);
			xfer_result (*recv)(socket_descriptor &sd, void *p, size_t n);
			void (*fin)(socket_descriptor &sd);
		};

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
			int descriptor() const { return pf->descriptor(sd); }
			std::string local_address() { return pf->local_address(sd); }
			std::string remote_address() { return pf->remote_address(sd); }
			accept_result accept();
			accept_result accept(std::string &addr_o);
			xfer_result send(void const *p, size_t n) { return pf->send(sd, p, n); }
			xfer_result send_all(void const *p, size_t n) { return pf->send_all(sd, p, n); }
			xfer_result recv(void *p, size_t n) { return pf->recv(sd, p, n); }
			void fin() { pf->fin(sd); }
		};

		struct accept_result {
			status stat;
			socket sock;
		};

		inline socket &socket::operator=(socket &&that) noexcept {
			swap(that);
			return *this;
		}

		inline void socket::swap(socket &that) noexcept {
			std::swap(pf, that.pf);
			std::swap(sd, that.sd);
		}

		inline accept_result socket::accept() {
		 	return pf->accept(sd, nullptr);
	 	}

		inline accept_result socket::accept(std::string &addr_o) {
		 	return pf->accept(sd, &addr_o);
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

		// default protocol family method implementations:
		void pf_unimpl_construct_descriptor(socket_descriptor &);
		void pf_unimpl_destruct_destriptor(socket_descriptor &);
		int pf_unimpl_descriptor(socket_descriptor const &);
		socket pf_unimpl_new_listener(std::string &, int);
		connect_result pf_unimpl_new_connection(std::string &);
		std::string pf_unimpl_local_address(socket_descriptor &);
		std::string pf_unimpl_remote_address(socket_descriptor &);
		accept_result pf_unimpl_accept(socket_descriptor &, std::string *);
		xfer_result pf_unimpl_send(socket_descriptor &, void const *, size_t);
		xfer_result pf_unimpl_send_all(socket_descriptor &, void const *, size_t);
		xfer_result pf_unimpl_recv(socket_descriptor &, void *, size_t);
		void pf_unimpl_fin(socket_descriptor &);
	}
}

#endif // #ifndef CLANE__NET_SOCKET_HPP
