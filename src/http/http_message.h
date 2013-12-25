// vim: set noet:

#ifndef CLANE_HTTP_MESSAGE_H
#define CLANE_HTTP_MESSAGE_H

/** @file
 *
 * @brief HTTP irequest and response types */

#include "http_common.h"
#include "http_header.h"
#include "../uri/uri.h"

namespace clane {
	namespace http {

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
			void clear() noexcept;
		};

		inline void request::clear() noexcept {
			method.clear();
			uri.clear();
			major_version = 0;
			minor_version = 0;
			headers.clear();
		}
	}
}

#endif // #ifndef CLANE_HTTP_MESSAGE_H
