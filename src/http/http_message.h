// vim: set noet:

#ifndef CLANE_HTTP_MESSAGE_H
#define CLANE_HTTP_MESSAGE_H

/** @file
 *
 * @brief HTTP request and response types */

#include "http_common.h"
#include <istream>
#include <ostream>

namespace clane {
	namespace http {

		class response_ostream: public std::ostream {
		public:
			virtual ~response_ostream() = default;
			response_ostream() = default;
			response_ostream(response_ostream const &) = delete;
			response_ostream(response_ostream &&) = default;
			response_ostream &operator=(response_ostream const &) = delete;
			response_ostream &operator=(response_ostream &&) = default;
		};

		class request {
		public:
			virtual ~request() = default;
			request() = default;
			request(request const &) = delete;
			request(request &&) = default;
			request &operator=(request const &) = delete;
			request &operator=(request &&) = default;
			//std::istream &body();
		};

	}
}

#endif // #ifndef CLANE_HTTP_MESSAGE_H
