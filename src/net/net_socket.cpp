// vim: set noet:

/** \file */

#include "net_error.h"
#include "net_socket.h"
#include <sstream>
#include <stdexcept>
#include <sys/socket.h>
#include <unistd.h>

namespace clane {
	namespace net {

		static char const *const pf_unsupported = "protocol family method is unsupported";

		char const *what(status stat) {
			switch (stat) {
				case status::ok: return "OK";
				case status::would_block: return "operation would block";
				case status::in_progress: return "operation is in progress";
				case status::timed_out: return "operation timed out";
				case status::conn_refused: return "connection was refused";
				case status::net_unreachable: return "network is unreachable";
				case status::reset: return "connection reset";
				case status::aborted: return "connection aborted";
				case status::no_resource: return "insufficient resources";
				default: return "unknown error";
			}
		}

		int sys_socket(int domain, int type, int protocol) {
			int fd = ::socket(domain, type, protocol);
			if (-1 == fd) {
				std::ostringstream ess;
				ess << "system socket creation error (domain=" << domain << ", type=" << type << ", protocol=" <<
					protocol << "): " << errno_to_string(errno);
				throw std::runtime_error(ess.str());
			}
			return fd;
		}

		void sys_bind(int sock, sockaddr const *addr, socklen_t addr_len) {
			int stat = ::bind(sock, addr, addr_len);
			if (-1 == stat) {
				std::ostringstream ess;
				ess << "system socket bind error: " << errno_to_string(errno);
				throw std::runtime_error(ess.str());
			}
		}

		void sys_listen(int sock, int backlog) {
			int stat = ::listen(sock, backlog);
			if (-1 == stat) {
				std::ostringstream ess;
				ess << "system socket listen error: " << errno_to_string(errno);
				throw std::runtime_error(ess.str());
			}
		}

		void sys_getsockname(int sock, sockaddr *addr, socklen_t addr_len) {
			socklen_t len = addr_len;
			int stat = ::getsockname(sock, addr, &len);
			if (-1 == stat) {
				std::ostringstream ess;
				ess << "system socket local address retrieval error (sock=" << sock << "): " << errno_to_string(errno);
				throw std::runtime_error(ess.str());
			}
			if (len > addr_len) {
				std::ostringstream ess;
				ess << "system socket local address retrieval error: address is " << len <<
						" bytes, expected no more than " << addr_len;
				throw std::runtime_error(ess.str());
			}
		}

		void sys_getpeername(int sock, sockaddr *addr, socklen_t addr_len) {
			socklen_t len = addr_len;
			int stat = ::getpeername(sock, addr, &len);
			if (-1 == stat) {
				std::ostringstream ess;
				ess << "system socket remote address retrieval error (sock=" << sock << "): " << errno_to_string(errno);
				throw std::runtime_error(ess.str());
			}
			if (len > addr_len) {
				std::ostringstream ess;
				ess << "system socket remote address retrieval error: address is " << len <<
						" bytes, expected no more than " << addr_len;
				throw std::runtime_error(ess.str());
			}
		}

		void sys_shutdown(int sock, int how) {
			int stat = ::shutdown(sock, how);
			if (-1 == stat) {
				std::ostringstream ess;
				ess << "system socket shutdown error: " << errno_to_string(errno);
				throw std::runtime_error(ess.str());
			}
		}

		int sys_getsockopt(int sock, int level, int optname) {
			int opt;
			socklen_t opt_len = sizeof(opt);
			int stat = ::getsockopt(sock, level, optname, &opt, &opt_len);
			if (-1 == stat) {
				std::ostringstream ess;
				ess << "system socket option retrieval error (sock=" << sock << ", level=" << level << ", optname=" << optname <<
					"): " << errno_to_string(errno);
				throw std::runtime_error(ess.str());
			}
			return opt;
		}

		void sys_setsockopt(int sock, int level, int optname, int val) {
			int stat = ::setsockopt(sock, level, optname, &val, sizeof(val));
			if (-1 == stat) {
				std::ostringstream ess;
				ess << "system socket option set error (sock=" << sock << ", level=" << level << ", optname=" << optname <<
					"): " << errno_to_string(errno);
				throw std::runtime_error(ess.str());
			}
		}

		void pf_unsupported_construct_instance(protocol_family::instance &) {
			throw std::logic_error(pf_unsupported);
		}

		void pf_unsupported_destruct_instance(protocol_family::instance &) {
			throw std::logic_error(pf_unsupported);
		}

		std::string pf_unsupported_local_address(protocol_family::instance &) {
			throw std::logic_error(pf_unsupported);
		}

		std::string pf_unsupported_remote_address(protocol_family::instance &) {
			throw std::logic_error(pf_unsupported);
		}

		connect_result pf_unsupported_connect(protocol_family::instance &, std::string &) {
			throw std::logic_error(pf_unsupported);
		}

		send_result pf_unsupported_send(protocol_family::instance &, void const *, size_t , int) {
			throw std::logic_error(pf_unsupported);
		}

		recv_result pf_unsupported_recv(protocol_family::instance &, void *, size_t , int) {
			throw std::logic_error(pf_unsupported);
		}

		protocol_family const *pf_unsupported_listen(protocol_family::instance &, std::string &, int) {
			throw std::logic_error(pf_unsupported);
		}

		accept_result pf_unsupported_accept(protocol_family::instance &, std::string *, int) {
			throw std::logic_error(pf_unsupported);
		}

		int pf_unsupported_domain() {
			throw std::logic_error(pf_unsupported);
		}

		socket::~socket() noexcept {
			if (pf)
				pf->destruct_instance(pi);
		}

		socket::socket(protocol_family const *pf, std::string addr): pf{}, pi{} {
			auto stat = connect_(pf, addr);
			if (status::ok != stat) {
				std::ostringstream ess;
				ess << "socket error: " << what(stat);
				throw std::runtime_error(ess.str());
			}
		}

		socket::socket(protocol_family const *pf, std::string addr, int backlog) {
			pf->construct_instance(pi);
			struct scoped_instance {
				protocol_family const *pf;
				socket *sock;
				~scoped_instance() { if (sock) { pf->destruct_instance(sock->pi); }}
			} instance_dtor{pf, this};
			this->pf = pf->listen(pi, addr, backlog);
			instance_dtor.sock = nullptr;
		}

		status socket::connect(protocol_family const *pf, std::string addr) {
			return connect_(pf, addr);
		}

		status socket::connect_(protocol_family const *pf, std::string &addr) {
			if (this->pf)
				throw std::logic_error("already connected");
			pf->construct_instance(pi);
			struct scoped_instance {
				protocol_family const *pf;
				socket *sock;
				~scoped_instance() { if (sock) { pf->destruct_instance(sock->pi); }}
			} instance_dtor{pf, this};
			auto result = pf->connect(pi, addr);
			if (status::ok != result.stat)
				return result.stat;
			this->pf = result.pf;
			instance_dtor.sock = nullptr;
			return status::ok;
		}

	}
}

