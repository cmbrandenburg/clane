// vim: set noet:

#ifndef CLANE__HTTP_MESSAGE_HPP
#define CLANE__HTTP_MESSAGE_HPP

#include "http_common.hpp"
#include "http_header.hpp"
#include "http_status.hpp"
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

		class response {
		public:
			int major_version;
			int minor_version;
			status_code status;
			std::string reason;
			header_map headers;
			std::istream body;
		public:
			~response() = default;
			response() = default;
			response(response const &) = delete;
			response(response &&) = default;
			response &operator=(response const &) = delete;
			response &operator=(response &&) = default;
		};

		bool parse_response(response &r, std::string const &s);
	}
}

#endif // #ifndef CLANE__HTTP_MESSAGE_HPP
