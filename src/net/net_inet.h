// vim: set noet:

#ifndef CLANE_NET_INET_H
#define CLANE_NET_INET_H

/** @file
 *
 * @brief Internet Protocol */

#include "net_common.h"
#include "net_socket.h"

namespace clane {
	namespace net {
		extern protocol_family const *tcp;
		extern protocol_family const *tcp4;
		extern protocol_family const *tcp6;
	}
}

#endif // #ifndef CLANE_NET_INET_H
