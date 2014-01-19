// vim: set noet:

#ifndef CLANE_NET_INET_HPP
#define CLANE_NET_INET_HPP

#include "net_common.hpp"

namespace clane {
	namespace net {

		struct protocol_family;
		extern protocol_family const tcp;
		extern protocol_family const tcp4;
		extern protocol_family const tcp6;
	}
}

#endif // #ifndef CLANE_NET_INET_HPP
