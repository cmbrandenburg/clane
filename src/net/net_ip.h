// vim: set noet:

#ifndef CLANE_NET_IP_H
#define CLANE_NET_IP_H

/** @file
 *
 * @brief IP-based implementations */

#include "net_common.h"
#include "net_socket.h"
#include <cstring>
#include <string>

namespace clane {
	namespace net {

		/** @brief Options for the listen_tcp() functions */
		struct tcp_listen_opts {

			/** @brief Whether to reuse the given address when binding the socket */
			bool reuse_addr;
		};

		/** @brief Creates a socket that's listening at the given address for
		 * incoming TCP connections */
		socket listen_tcp(std::string const &addr, int backlog, tcp_listen_opts const *opts = nullptr);

		/** @brief Creates a socket that's listening at the given address for
		 * incoming TCP connections */
		socket listen_tcp(char const *addr, int backlog, tcp_listen_opts const *opts = nullptr);

		/** @brief Creates a socket that's listening at the given address for
		 * incoming TCP connections */
		socket listen_tcp(char const *addr, size_t addr_len, int backlog, tcp_listen_opts const *opts);

		/** @brief Options for the dial_tcp() functions */
		struct tcp_dial_opts {
		};

		/** @brief Creates a socket that's connected to the given address via TCP */
		socket dial_tcp(std::string const &addr, tcp_dial_opts const *opts = nullptr);

		/** @brief Creates a socket that's connected to the given address via TCP */
		socket dial_tcp(char const *addr, tcp_dial_opts const *opts = nullptr);

		/** @brief Creates a socket that's connected to the given address via TCP */
		socket dial_tcp(char const *addr, size_t addr_len, tcp_dial_opts const *opts);

		inline socket dial_tcp(std::string const &addr, tcp_dial_opts const *opts) {
			return dial_tcp(addr.c_str(), addr.size(), opts);
		}

		inline socket dial_tcp(char const *addr, tcp_dial_opts const *opts) {
			return dial_tcp(addr, strlen(addr), opts);
		}

		inline socket listen_tcp(std::string const &addr, int backlog, tcp_listen_opts const *opts) {
			return listen_tcp(addr.c_str(), addr.size(), backlog, opts);
		}

		inline socket listen_tcp(char const *addr, int backlog, tcp_listen_opts const *opts) {
			return listen_tcp(addr, strlen(addr), backlog, opts);
		}
	}
}

#endif // #ifndef CLANE_NET_IP_H
