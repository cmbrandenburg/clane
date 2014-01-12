// vim: set noet:

#ifndef CLANE__HTTP_REQUEST_HPP
#define CLANE__HTTP_REQUEST_HPP

#include "http_common.hpp"
#include "http_header.hpp"
#include "../uri/uri.hpp"
#include <istream>

namespace clane {
	namespace http {

		class request {
		public:
			std::string method;
			uri::uri uri;
			int major_version;
			int minor_version;
			header_map headers;
		public:
			~request() = default;
			request() = default;
			request(request const &) = default;
			request(request &&) = default;
			request &operator=(request const &) = default;
			request &operator=(request &&) = default;
		};
	}
}

#endif // #ifndef CLANE__HTTP_REQUEST_HPP
