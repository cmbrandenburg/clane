// vim: set noet:

#ifndef CLANE_HTTP_REQUEST_H
#define CLANE_HTTP_REQUEST_H

/** @file
 *
 * @brief HTTP request */

#include "http_common.h"
#include <istream>

namespace clane {
	namespace http {

		class request: public std::istream {
		public:
			~request() = default;
			request(std::streambuf *sb);
			request(request const &) = delete;
			request(request &&) = default;
			request &operator=(request const &) = delete;
			request &operator=(request &&) = default;
		};
	}
}

#endif // #ifndef CLANE_HTTP_REQUEST_H
