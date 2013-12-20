// vim: set noet:

/** @file */

#include "http_status.h"

namespace clane {
	namespace http {

		std::ostream &operator<<(std::ostream &ostrm, status_code val) {
			return ostrm << static_cast<typename std::underlying_type<status_code>::type>(val);
		}
	}
}

