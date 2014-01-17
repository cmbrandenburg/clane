// vim: set noet:

#ifndef CLANE__HTTP_RESPONSE_HPP
#define CLANE__HTTP_RESPONSE_HPP

#include "http_common.hpp"
#include "http_header.hpp"
#include "http_status.hpp"
#include <istream>

namespace clane {
	namespace http {

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

#endif // #ifndef CLANE__HTTP_RESPONSE_HPP
