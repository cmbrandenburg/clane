// vim: set noet:

#include "clane_posix.hpp"
#include "clane_socket.hpp"
#include <sstream>
#include <stdexcept>
#include <unistd.h>

namespace clane {
	namespace net {

		posix::file_descriptor sys_socket(int domain, int type, int protocol) {
			posix::file_descriptor sock = ::socket(domain, type, protocol);
			if (-1 == sock) {
				std::ostringstream ess;
				ess << "open socket (domain=" << domain << ", type=" << type << ", protocol=" << protocol << "): " <<
					posix::errno_to_string(errno);
				throw std::runtime_error(ess.str());
			}
			return sock;
		}

		void sys_setsockopt(int sockfd, int level, int optname, int optval) {
			int stat = ::setsockopt(sockfd, level, optname, &optval, sizeof(optval));
			if (-1 == stat) {
				std::ostringstream ess;
				ess << "set socket option (sockfd=" << sockfd << ", level=" << level << ", optname=" << optname <<
					", optval=" << optval << "): " << posix::errno_to_string(errno);
				throw std::runtime_error(ess.str());
			}
		}

		void sys_bind(int sockfd, sockaddr const *addr, socklen_t addr_len) {
			int stat = ::bind(sockfd, addr, addr_len);
			if (-1 == stat) {
				std::ostringstream ess;
				ess << "bind socket (sockfd=" << sockfd << "): " << posix::errno_to_string(errno);
				throw std::runtime_error(ess.str());
			}
		}

		void sys_listen(int sockfd, int backlog) {
			int stat = ::listen(sockfd, backlog);
			if (-1 == stat) {
				std::ostringstream ess;
				ess << "listen socket (sockfd=" << sockfd << "): " << posix::errno_to_string(errno);
				throw std::runtime_error(ess.str());
			}
		}

		void sys_getsockname(int sockfd, sockaddr *addr, socklen_t addr_len) {
			socklen_t len = addr_len;
			int stat = ::getsockname(sockfd, addr, &len);
			if (-1 == stat) {
				std::ostringstream ess;
				ess << "get socket name (sockfd=" << sockfd << "): " << posix::errno_to_string(errno);
				throw std::runtime_error(ess.str());
			}
			if (len > addr_len) {
				std::ostringstream ess;
				ess << "get socket name: address is " << len << " bytes, expected no more than " << addr_len;
				throw std::runtime_error(ess.str());
			}
		}

		void sys_getpeername(int sockfd, sockaddr *addr, socklen_t addr_len) {
			socklen_t len = addr_len;
			int stat = ::getpeername(sockfd, addr, &len);
			if (-1 == stat) {
				std::ostringstream ess;
				ess << "get socket peer (sockfd=" << sockfd << "): " << posix::errno_to_string(errno);
				throw std::runtime_error(ess.str());
			}
			if (len > addr_len) {
				std::ostringstream ess;
				ess << "get socket peer: address is " << len << " bytes, expected no more than " << addr_len;
				throw std::runtime_error(ess.str());
			}
		}

		status sys_connect(int sockfd, sockaddr const *addr, socklen_t addr_len) {
			int stat = TEMP_FAILURE_RETRY(::connect(sockfd, addr, addr_len));
			if (-1 == stat) {
				switch (errno) {
					case EACCES: case EPERM: return status::permission;
					case EAGAIN: return status::no_resource;
					case ECONNREFUSED: return status::conn_refused;
					case EINPROGRESS: return status::in_progress;
					case ENETUNREACH: return status::net_unreachable;
					case ETIMEDOUT: return status::timed_out;
					default: {
						std::ostringstream ess;
						ess << "connect (sockfd=" << sockfd << "): " << posix::errno_to_string(errno);
						throw std::runtime_error(ess.str());
					}
				}
			}
			return status::ok;
		}

		static char const *const pf_unsupported = "protocol family method is unsupported";

		// unimplemented method placeholders:
		void pf_unimpl_construct_descriptor(socket_descriptor &) { throw std::logic_error(pf_unsupported); }
		void pf_unimpl_destruct_destriptor(socket_descriptor &) { throw std::logic_error(pf_unsupported); }
		socket pf_unimpl_new_listener(std::string &, int) { throw std::logic_error(pf_unsupported); }
		connect_result pf_unimpl_new_connection(std::string &) { throw std::logic_error(pf_unsupported); }
		std::string pf_unimpl_local_address(socket_descriptor &) { throw std::logic_error(pf_unsupported); }
		std::string pf_unimpl_remote_address(socket_descriptor &) { throw std::logic_error(pf_unsupported); }
	}
}

