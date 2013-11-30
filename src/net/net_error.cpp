// vim: set noet:

/** \file */

#include "net_error.h"
#include <cstring>

namespace clane {
	namespace net {

		std::string errno_to_string(int e) {
			char buf[256];
			return strerror_r(e, buf, sizeof(buf));
		}
	}
}

