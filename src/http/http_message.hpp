// vim: set noet:

#ifndef CLANE__HTTP_MESSAGE_HPP
#define CLANE__HTTP_MESSAGE_HPP

#include "http_common.hpp"
#include "http_header.hpp"
#include "http_status.hpp"
#include "../uri/uri.hpp"
#include <istream>
#include <memory>

namespace clane {
	namespace http {

		// Base class for server-side and client-side body stream buffers. This
		// provides an additional method to add more input to the input sequence.
		class streambuf: public std::streambuf {
		public:
			virtual ~streambuf();
			streambuf();
			streambuf(streambuf const &) = default;
			#ifndef _WIN32
			streambuf(streambuf &&) = default;
			#endif
			streambuf &operator=(streambuf const &) = default;
			#ifndef _WIN32
			streambuf &operator=(streambuf &&) = default;
			#endif
			virtual void more_input(std::shared_ptr<char> const &p, size_t offset, size_t size) = 0;
		};

		class request {
		public:
			std::string method;
			uri::uri uri;
			int major_version;
			int minor_version;
			header_map headers;
			header_map trailers;
			//std::istream body;
		public:
			~request() = default;
			request() = default;
			request(request const &) = default;
			#ifndef _WIN32
			request(request &&) = default;
			#endif
			request &operator=(request const &) = default;
			#ifndef _WIN32
			request &operator=(request &&) = default;
			#endif
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
			#ifndef _WIN32
			response(response &&) = default;
			#endif
			response &operator=(response const &) = delete;
			#ifndef _WIN32
			response &operator=(response &&) = default;
			#endif
		};

		bool parse_response(response &r, std::string const &s);
	}
}

#endif // #ifndef CLANE__HTTP_MESSAGE_HPP
