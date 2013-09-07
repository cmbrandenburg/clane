// vim: set noet:

#ifndef CLANE_NET_ERROR_H
#define CLANE_NET_ERROR_H

/** @file
 *
 * @brief Common error-handling */

#include "net_common.h"
#include <string>

namespace clane {
	namespace net {

		/** @brief Thread-safe `strerror` */
		std::string safe_strerror(int e);
	}
}

#endif // #ifndef CLANE_NET_ERROR_H
