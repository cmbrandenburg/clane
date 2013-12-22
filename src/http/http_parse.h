// vim: set noet:

#ifndef CLANE_HTTP_PARSE_H
#define CLANE_HTTP_PARSE_H

/** @file
 *
 * @brief HTTP parsing */

#include "http_common.h"
#include "http_header.h"
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

		protected:

			// Increases the current length and returns true if and only if the
			// current length limit is not set or not exceeded.
			bool increase_length(size_t n);

			// Sets the parser into the error state, with a description of the error.
			void set_error(status_code error_stat, char const *what);
		};

		class headers_parser: public parser {
		private:
			enum class phase {
				start_line,   // after consuming newline, expecting header name or linear whitespace
				end_newline,  // expecting newline character to end headers
				name,         // consuming a header name
				pre_value,    // whitespace before header value, or linear whitespace in line continuation
				value,        // consuming a header value
				value_newline // expecting newline character to end value line
			} cur_phase;
		public:
			header_map hdrs;
		private:
			std::string hdr_name;
			std::string hdr_val;
		public:
			~headers_parser() = default;
			headers_parser();
			headers_parser(headers_parser const &) = default;
			headers_parser(headers_parser &&) = default;
			headers_parser &operator=(headers_parser const &) = default;
			headers_parser &operator=(headers_parser &&) = default;
			bool parse(char const *buf, size_t size);

			void reset();
		};

		class request_line_parser: public parser {
		private:
			enum class phase {
				method,
				uri,
				version,
				newline
			} cur_phase;
		public:
			std::string method;
			uri::uri uri;
			std::string uri_str;
			int major_ver;
			int minor_ver;
		private:
			std::string version_str;
		public:
			~request_line_parser() = default;
			request_line_parser();
			request_line_parser(request_line_parser const &) = default;
			request_line_parser(request_line_parser &&) = default;
			request_line_parser &operator=(request_line_parser const &) = default;
			request_line_parser &operator=(request_line_parser &&) = default;

			/** Parses
			 *
			 * @return Returns true if and only if parsing completes without error */
			bool parse(char const *buf, size_t size);

			void reset();

		private:
			bool parse_version();
		};

		class request_1x_parser: public parser {
		private:
			enum class phase {
				request_line,
				headers
			} cur_phase;
		public:
			request_line_parser req_line_parser;
			headers_parser hdrs_parser;
		public:
			~request_1x_parser() = default;
			request_1x_parser();
			request_1x_parser(request_1x_parser const &) = default;
			request_1x_parser(request_1x_parser &&) = default;
			request_1x_parser &operator=(request_1x_parser const &) = default;
			request_1x_parser &operator=(request_1x_parser &&) = default;
			bool parse(char const *buf, size_t size);
			void reset();
		};
	}
}

#endif // #ifndef CLANE_HTTP_PARSE_H
