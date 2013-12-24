// vim: set noet:

#ifndef CLANE_HTTP_PARSE_H
#define CLANE_HTTP_PARSE_H

/** @file
 *
 * @brief HTTP parsing */

#include "http_common.h"
#include "http_header.h"
#include "http_message.h"
#include "http_status.h"
#include "../uri/uri.h"

namespace clane {
	namespace http {

		// Base class for all parsers.
		class parser {
		protected:
			enum class status {
				ready, // ready to parse (more) data
				ok,    // parsing completed with success
				error  // parsing completed with error
			} stat;
		private:
			size_t len_limit; // length limit, if any
			size_t cur_len;
			status_code error_code_; // HTTP response status, in case an error occurred
			char const *what_;       // error description, in case an error occurred
		public:
			~parser() = default;
			parser();
			parser(parser const &) = default;
			parser(parser &&) = default;
			parser &operator=(parser const &) = default;
			parser &operator=(parser &&) = default;

			// Returns true if and only if the parser is in a non-error state.
			operator bool() const { return stat != status::error; }

			// Returns the HTTP response status, if an error occurred.
			status_code error_code() const { return error_code_; }

			// Returns the error description, if an error occurred.
			char const *what() const { return what_; }

			// Resets the parser state so that it may begin parsing anew.
			void reset();

			// Returns the number of bytes consumed from parsing.
			size_t parse_length() const { return cur_len; }

			// Sets the parser's length limit.
			void set_length_limit(size_t n) { len_limit = n; }

		protected:

			// Increases the current length and returns true if and only if the
			// current length limit is not set or not exceeded.
			bool increase_length(size_t n);

			// Sets the parser into the error state, with a description of the error.
			void set_error(status_code error_stat, char const *what);
		};

		class headers_parser: virtual public parser {
			enum class phase {
				start_line,   // after consuming newline, expecting header name or linear whitespace
				end_newline,  // expecting newline character to end headers
				name,         // consuming a header name
				pre_value,    // whitespace before header value, or linear whitespace in line continuation
				value,        // consuming a header value
				value_newline // expecting newline character to end value line
			} cur_phase;
			header_map *hdrs;
			std::string hdr_name;
			std::string hdr_val;
		public:
			~headers_parser() = default;
			headers_parser(header_map &hdrs);
			headers_parser(headers_parser const &) = default;
			headers_parser(headers_parser &&) = default;
			headers_parser &operator=(headers_parser const &) = default;
			headers_parser &operator=(headers_parser &&) = default;
			bool parse(char const *buf, size_t size);
			void reset(header_map &hdrs);
		};

		class request_line_parser: virtual public parser {
			enum class phase {
				method,
				uri,
				version,
				newline
			} cur_phase;
			std::string *method;
			uri::uri *uri;
			int *major_ver;
			int *minor_ver;
			std::string uri_str;
			std::string version_str;
		public:
			~request_line_parser() = default;
			request_line_parser(std::string &method, uri::uri &uri, int &major_ver, int &minor_ver);
			request_line_parser(request_line_parser const &) = default;
			request_line_parser(request_line_parser &&) = default;
			request_line_parser &operator=(request_line_parser const &) = default;
			request_line_parser &operator=(request_line_parser &&) = default;
			bool parse(char const *buf, size_t size);
			void reset(std::string &method, uri::uri &uri, int &major_ver, int &minor_ver);
		private:
			bool parse_version();
		};

		class request_1x_parser: virtual public parser, private request_line_parser, private headers_parser {
			enum class phase {
				request_line,
				headers
			} cur_phase;
		public:
			~request_1x_parser() = default;
			request_1x_parser(request &req);
			request_1x_parser(request_1x_parser const &) = default;
			request_1x_parser(request_1x_parser &&) = default;
			request_1x_parser &operator=(request_1x_parser const &) = default;
			request_1x_parser &operator=(request_1x_parser &&) = default;
			bool parse(char const *buf, size_t size);
			void reset(request &req);
		};

		inline headers_parser::headers_parser(header_map &hdrs): cur_phase(phase::start_line), hdrs(&hdrs) {
		}

		inline void headers_parser::reset(header_map &hdrs) {
			parser::reset();
			cur_phase = phase::start_line;
			this->hdrs = &hdrs;
			hdr_name.clear();
			hdr_val.clear();
		}

		inline request_line_parser::request_line_parser(std::string &method, uri::uri &uri, int &major_ver, int &minor_ver):
		  	 	cur_phase(phase::method), method(&method), uri(&uri), major_ver(&major_ver), minor_ver(&minor_ver) {
		}

		inline void request_line_parser::reset(std::string &method, uri::uri &uri, int &major_ver, int &minor_ver) {
			parser::reset();
			cur_phase = phase::method;
			this->method = &method;
			this->uri = &uri;
			this->major_ver = &major_ver;
			this->minor_ver = &minor_ver;
			uri_str.clear();
			version_str.clear();
		}

		inline request_1x_parser::request_1x_parser(request &req):
		  	 	request_line_parser(req.method, req.uri, req.major_version, req.minor_version),
				headers_parser(req.headers), cur_phase(phase::request_line) {
		}

		inline void request_1x_parser::reset(request &req) {
			parser::reset();
			cur_phase = phase::request_line;
			request_line_parser::reset(req.method, req.uri, req.major_version, req.minor_version);
			headers_parser::reset(req.headers);
		}

	}
}

#endif // #ifndef CLANE_HTTP_PARSE_H
