// vim: set noet:

/** @file */

#include "http_request.h"

namespace clane {
	namespace http {

		request::request(std::streambuf *sb): std::istream(sb) {
		}
	}
}

