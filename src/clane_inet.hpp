// vim: set noet:

#ifndef CLANE__INET_HPP
#define CLANE__INET_HPP

#include "clane_common.hpp"

namespace clane {
	namespace net {

		class protocol_family;
		extern protocol_family const tcp;
		extern protocol_family const tcp4;
		extern protocol_family const tcp6;
	}
}

#endif // #ifndef CLANE__INET_HPP
