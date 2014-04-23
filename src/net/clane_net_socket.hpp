// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// vim: set noet:

#ifndef CLANE__NET_SOCKET_HPP
#define CLANE__NET_SOCKET_HPP

/** @file */

#include "../clane_base.hpp"
#include "../posix/clane_posix_fd.hpp"
#include "../../include/clane_net.hpp"
#include <sys/socket.h>
#include <sys/types.h>

namespace clane {
	namespace net {

		// low-level socket functions:
		posix::unique_fd sys_socket(int domain, int type, int protocol);
		void sys_setsockopt(int sock_fd, int level, int optname, int val);
		void sys_bind(int sockfd, sockaddr const *addr, socklen_t addr_len);
		void sys_listen(int sockfd, int backlog);
		void sys_getsockname(int sockfd, sockaddr *addr, socklen_t addr_len);
		void sys_getpeername(int sockfd, sockaddr *addr, socklen_t addr_len);
		void sys_connect(int sockfd, sockaddr const *addr, socklen_t addr_len, std::error_code &e);
		posix::unique_fd sys_accept(int sockfd, sockaddr *addr, socklen_t addr_len, std::error_code &e);

#define pf_unsupported() throw std::logic_error("protocol family method is unsupported");

		// default protocol family method implementations:
		inline void pf_unimpl_construct_descriptor(socket_descriptor &) { pf_unsupported(); }
		inline void pf_unimpl_destruct_destriptor(socket_descriptor &) { pf_unsupported(); }
		inline int pf_unimpl_descriptor(socket_descriptor const &) { pf_unsupported(); }
		inline socket pf_unimpl_new_listener(std::string &, int) { pf_unsupported(); }
		inline socket pf_unimpl_new_connection(std::string &, std::error_code &) { pf_unsupported(); }
		inline void pf_unimpl_set_nonblocking(socket_descriptor &) { pf_unsupported(); }
		inline std::string pf_unimpl_local_address(socket_descriptor &) { pf_unsupported(); }
		inline std::string pf_unimpl_remote_address(socket_descriptor &) { pf_unsupported(); }
		inline socket pf_unimpl_accept(socket_descriptor &, std::string *, std::error_code &) { pf_unsupported(); }
		inline size_t pf_unimpl_send(socket_descriptor &, void const *, size_t, int, std::error_code &) { pf_unsupported(); }
		inline size_t pf_unimpl_recv(socket_descriptor &, void *, size_t, int, std::error_code &) { pf_unsupported(); }
		inline void pf_unimpl_fin(socket_descriptor &) { pf_unsupported(); }
	}
}

#endif // #ifndef CLANE__NET_SOCKET_HPP
