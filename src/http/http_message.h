// vim: set noet:

#ifndef CLANE_HTTP_MESSAGE_H
#define CLANE_HTTP_MESSAGE_H

/** @file
 *
 * @brief HTTP irequest and response types */

#include "http_common.h"
#include "http_header.h"
#include "../uri/uri.h"
#include <istream>
#include <ostream>

namespace clane {
	namespace http {

		class oresponse: public std::ostream {
		public:
			virtual ~oresponse() = default;
			oresponse() = default;
			oresponse(oresponse const &) = delete;
			oresponse(oresponse &&) = default;
			oresponse &operator=(oresponse const &) = delete;
			oresponse &operator=(oresponse &&) = default;
		};

		class request {
		public:
			std::string method;
			uri::uri uri;
			int major_version;
			int minor_version;
			header_map headers;
			~request() = default;
			request() = default;
			request(request const &) = default;
			request(request &&) = default;
			request &operator=(request const &) = default;
			request &operator=(request &&) = default;
		};

		class irequest: public request, public std::istream {
		public:
			virtual ~irequest() = default;
			irequest() = default;
			irequest(irequest const &) = delete;
			irequest(irequest &&) = default;
			irequest &operator=(irequest const &) = delete;
			irequest &operator=(irequest &&) = default;
		};

	}
}

#endif // #ifndef CLANE_HTTP_MESSAGE_H
