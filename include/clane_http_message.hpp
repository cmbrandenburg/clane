// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// vim: set noet:

#ifndef CLANE_HTTP_MESSAGE_HPP
#define CLANE_HTTP_MESSAGE_HPP

/** @file */

#include "clane_ascii.hpp"
#include "clane_base.hpp"
#include "clane_uri.hpp"
#include <istream>
#include <map>
#include <memory>

namespace clane {

	namespace http {

		/** @page http_request_handling_page HTTP request handling
		 *
		 * @remark Server applications respond to incoming HTTP requests by using
		 * one or more **HTTP request handlers**, or **handlers** for short. A
		 * handler is a callable object—e.g., function, functor, `std::function`,
		 * etc.—with the following signature:
		 *
		 * @remark @code void(response_ostream &rs request &req) @endcode
		 *
		 * @remark The handler receives request input via the @p req argument and
		 * emits response output via the @p rs argument. By default, the response
		 * has a status code of 200 “OK” and contains no headers and no content. The
		 * handler may change the status code, add headers, and add content.
		 *
		 * @remark Server applications are event-driven; a basic_server instance
		 * calls its root handler once for each valid request it receives.
		 * Applications may use custom handlers as well as using built-in handlers
		 * provided by @projectname. Because a basic_server instance allows for only
		 * one **root handler**, applications that use multiple handlers must invoke
		 * handlers from other handlers, starting with the root handler.
		 * @projectname provides the basic_router class, itself a handler, as a
		 * means of dispatching an incoming request to one of many handlers by
		 * matching the incoming request against a set of criteria provided by the
		 * application. For example, an application may create a router to dispatch
		 * `GET` requests with a URI path starting with `/files/` to handler A,
		 * `POST` requests with a URI path starting with `/api/` to handler B, and
		 * serve all other requests a 404 “Not Found” error. Chaining handlers
		 * together like so leads applications towards a simple yet powerful design.
		 *
		 * @remark Another built-in handler is the file_server class, which serves
		 * static content from the server's file system—e.g., `.html`, `.css`, `.js`
		 * files, etc. The file server serves both regular files and directory
		 * listings.
		 *
		 * @remark To find out more about built-in handlers, consult the specific
		 * documentation for those types.
		 *
		 * @par Built-in request handlers
		 *
		 * @li basic_prefix_stripper
		 * @li basic_router
		 * @li file_server
		 *
		 * @sa basic_server
		 *
		 */

		/** @brief HTTP status code */
		enum class status_code {

			cont = 100,
			switching_protocols = 101,

			ok = 200,
			created = 201,
			accepted = 202,
			non_authoritative_information = 203,
			no_content = 204,
			reset_content = 205,
			partial_content = 206,

			multiple_choices = 300,
			moved_permanently = 301,
			found = 302,
			see_other = 303,
			not_modified = 304,
			use_proxy = 305,
			temporary_redirect = 307,

			bad_request = 400,
			unauthorized = 401,
			payment_required = 402,
			forbidden = 403,
			not_found = 404,
			method_not_allowed = 405,
			not_acceptable = 406,
			proxy_authentication_required = 407,
			request_timeout = 408,
			conflict = 409,
			gone = 410,
			length_required = 411,
			precondition_failed = 412,
			request_entity_too_large = 413,
			request_uri_too_long = 414,
			unsupported_media_type = 415,
			requested_range_not_satisfiable = 416,
			expectation_failed = 417,

			internal_server_error = 500,
			not_implemented = 501,
			bad_gateway = 502,
			service_unavailable = 503,
			gateway_timeout = 504,
			http_version_not_supported = 505
		};

		/** @brief Returns a human-readable string for a given HTTP status code
		 * */
		char const *what(status_code n);

		/** @brief Returns whether an HTTP status code denotes a client or server
		 * error */
		inline bool denotes_error(status_code n) {
			return 400 <= static_cast<int>(n) && static_cast<int>(n) < 600;
		};

		class header_name_less {
		public:
			bool operator()(std::string const &a, std::string const &b) const { return clane::ascii::icase_compare(a, b) < 0; }
		};

		class header_equal {
		public:
			bool operator()(std::string const &a, std::string const &b) const { return clane::ascii::icase_compare(a, b) == 0; }
		};

		/** @brief Map type for pairing HTTP header names to header values
		 *
		 * @remark Header names are case-insensitive, and header values are case
		 * sensitive. */
		typedef std::multimap<std::string, std::string, header_name_less> header_map;

		/** @brief HTTP header name–value pair
		 *
		 * @remark Header names are case-insensitive, and header values are case
		 * sensitive. */
		typedef header_map::value_type header;

		inline bool header_equal(header const &a, header const &b) {
			return clane::ascii::icase_compare(a.first, b.first) == 0 && a.second == b.second;
		}

		inline bool header_less(header const &a, header const &b) {
			int n = clane::ascii::icase_compare(a.first, b.first);
			return n < 0 || (n == 0 && a.second < b.second);
		}

		inline bool header_less_equal(header const &a, header const &b) {
			int n = clane::ascii::icase_compare(a.first, b.first);
			return n < 0 || (n == 0 && a.second <= b.second);
		}

		inline bool operator==(header const &a, header const &b) { return header_equal(a, b); }
		inline bool operator!=(header const &a, header const &b) { return !header_equal(a, b); }
		inline bool operator<(header const &a, header const &b) { return header_less(a, b); }
		inline bool operator<=(header const &a, header const &b) { return header_less_equal(a, b); }
		inline bool operator>(header const &a, header const &b) { return !header_less_equal(a, b); }
		inline bool operator>=(header const &a, header const &b) { return !header_less(a, b); }

		bool operator==(header_map const &a, header_map const &b);
		inline bool operator!=(header_map const &a, header_map const &b) { return !(a == b); }
		bool operator<(header_map const &a, header_map const &b);
		inline bool operator<=(header_map const &a, header_map const &b) { return a == b || a < b; }
		inline bool operator>(header_map const &a, header_map const &b) { return !(a <= b); }
		inline bool operator>=(header_map const &a, header_map const &b) { return !(a < b); }

		inline bool operator==(header_map const &a, header_map const &b) {
			return a.size() == b.size() && std::equal(a.begin(), a.end(), b.begin(), header_equal);
		}

		inline bool operator<(header_map const &a, header_map const &b) {
			return std::lexicographical_compare(a.begin(), a.end(), b.begin(), b.end(), header_less);
		}

		/** @brief Base class for all incremental parsers
		 *
		 * @remark An incremental parser processes a stream of input, one memory
		 * block at a time. Parsers are implemented as derived classes that follow
		 * idioms rather than overriding `virtual` functions.
		 *
		 * @remark Derived classes implement a non-virtual
		 * <code>parse_some</code> member function that accepts a memory block
		 * as input and returns the number of bytes parsed, or else on error
		 * sets the error state and returns 0. After the parse operation, the
		 * parser may be checked for whether parsing completed. For example, an
		 * HTTP header parser will be incomplete if the input memory block
		 * contains only a partial header (e.g., <code>"Content-Length:
		 * 0\r\nContent-"</code>), whereas the parser will be complete if the
		 * input memory block contains a full header (e.g.,
		 * <code>"Content-Length: 0\r\nContent-Type:
		 * text/plain\r\n\r\n"</code>).
		 *
		 * @remark Parsing may result in only some of the input memory block being
		 * parsed. This will happen if the input memory block contains additional
		 * data (e.g., an HTTP header parser will ignore entity body data following
		 * the end of the header) or if (by design) the parser returns control to
		 * the caller to allow the caller to process the data parsed so far before
		 * resuming further parsing. 
		 *
		 * @remark A parser instance may be reused after calling its
		 * <code>reset</code> method. Derived classes overload this method to
		 * carry out additional reset steps.
		 *
		 * @remark The parser class uses no virtual methods. The base class is
		 * provided as a means of providing state common to all parser subclasses;
		 * otherwise the parsers are unrelated to each other except by idiom. */
		class incparser {
			size_t cur_len;
			size_t lim_len;
			bool ready;
			status_code ecode;
			char const *ewhat;
		public:

			/** @brief Parsing return value when error has occurred */
			static size_t const error = static_cast<size_t>(-1);

			~incparser() {}

			/** @brief Constructs a non-parse-ready parser
			 *
			 * @remark Call the reset() function to prepare the parser for
			 * parsing. */
			incparser();

			incparser(incparser const &) = default;
			incparser &operator=(incparser const &) = default;
#ifndef CLANE_HAVE_NO_DEFAULT_MOVE
			// Move not implemented on all platforms because currently it's unused.
			incparser(incparser &&) = default;
			incparser &operator=(incparser &&) = default;
#endif

			/** @brief Returns whether the parser is ready to parse
			 *
			 * @remark A parser is ready to parse if and only if it's neither in
			 * the _completed_ state or an _error_ state. */
			operator bool() const { return ready; }

			/** @brief If a parsing error has occurred, returns a relevant HTTP
			 * status code describing the error
			 *
			 * @remark If the parsing error occurs in a client-specific parser,
			 * such as status line parser, then the status code is meaningless.
			 * Client-specific parsing doesn't categorize errors. */
			status_code status() const { return ecode; }

			/** @brief If a parsing error has occurred, return a human-readable
			 * description of the error */
			char const *what() const { return ewhat; }

			/** @brief Prepares the parser to parse
			 *
			 * @remark The reset() function allows a parser instance to be reused. */
			void reset();

			/** @brief Sets the length limit, or `0` for no limit.
			 *
			 * @remark Some parsers allow for a length limit. If a nonzero length
			 * limit `N` is set then parsing will fail upon being requested to parser
			 * more than `N` cumulative bytes. This is useful for security, so as to
			 * prevent an untrusted source to overfill the parser. */
			void set_length_limit(size_t n);

		protected:

			/** @brief Increases the current number of bytes parsed
			 *
			 * @return The increase_length() function returns true if and only if the
			 * length limit is unset or not exceeded. */
			bool increase_length(size_t n);

			/** @brief Sets the parser into the _completed_ state */
			void set_done();

			/** @brief Sets the parser into an _error_ state */
			void set_error(status_code ecode, char const *ewhat);
		};

		inline incparser::incparser(): cur_len{}, lim_len{}, ready{} {}

		inline bool incparser::increase_length(size_t n) {
			size_t new_len = cur_len + n;
			if (new_len < cur_len)
				return false; // overflow
			if (lim_len && new_len > lim_len)
				return false; // length limit set and exceeded
			cur_len = new_len;
			return true;
		}

		inline void incparser::reset() {
			ready = true;
			cur_len = 0;
			// The length limit is preserved.
		}

		inline void incparser::set_done() {
			ready = false;
		}

		inline void incparser::set_length_limit(size_t n) {
		 	lim_len = n;
	 	}

		inline void incparser::set_error(status_code ecode, char const *ewhat) {
			ready = false;
			this->ecode = ecode;
			this->ewhat = ewhat;
		}

		class v1x_request_line_incparser: virtual public incparser {
			enum class state {
				method,
				uri,
				version,
				newline
			} cur_stat;
			std::string method_;
			uri::uri uri_;
			int major_ver;
			int minor_ver;
			std::string uri_str;
			std::string version_str;
		public:
			~v1x_request_line_incparser() {}
			v1x_request_line_incparser() {}
			v1x_request_line_incparser(v1x_request_line_incparser const &) = default;
			v1x_request_line_incparser &operator=(v1x_request_line_incparser const &) = default;
#ifndef CLANE_HAVE_NO_DEFAULT_MOVE
			v1x_request_line_incparser(v1x_request_line_incparser &&) = default;
			v1x_request_line_incparser &operator=(v1x_request_line_incparser &&) = default;
#endif

			void reset();
			size_t parse_some(char const *beg, char const *end);

			// accessors:
			std::string const &method() const { return method_; }
			std::string &method() { return method_; }
			uri::uri const &uri() const { return uri_; }
			uri::uri &uri() { return uri_; }
			int major_version() const { return major_ver; }
			int minor_version() const { return minor_ver; }
		};

		class v1x_status_line_incparser: virtual public incparser {
			enum class state {
				version,
				status,
				reason,
				newline
			} cur_stat;
			int major_ver;
			int minor_ver;
			status_code status_;
			std::string reason_;
			std::string version_str;
			std::string status_str;
		public:
			~v1x_status_line_incparser() {}
			v1x_status_line_incparser() {}
			v1x_status_line_incparser(v1x_status_line_incparser const &) = default;
			v1x_status_line_incparser &operator=(v1x_status_line_incparser const &) = default;
#ifndef CLANE_HAVE_NO_DEFAULT_MOVE
			v1x_status_line_incparser(v1x_status_line_incparser &&) = default;
			v1x_status_line_incparser &operator=(v1x_status_line_incparser &&) = default;
#endif

			void reset();
			size_t parse_some(char const *beg, char const *end);

			// accessors:
			int major_version() const { return major_ver; }
			int minor_version() const { return minor_ver; }
			status_code status() const { return status_; }
			std::string const &reason() const { return reason_; }
			std::string &reason() { return reason_; }
		};

		class v1x_headers_incparser: virtual public incparser {
			enum class state {
				start_line,      // expecting a header or an empty line
				end_newline,     // expecting newline character to end headers
				name,            // consuming a header name
				value_skipws,    // whitespace before header value, or linear whitespace in line continuation
				value,           // consuming a header value
				value_newline,   // expecting newline character to end value line
				value_start_line // after a newline after a name-value pair, expecting linear whitespace, new header, or empty line
			} cur_stat;
			header_map hdrs;
			std::string hdr_name;
			std::string hdr_val;
		public:
			~v1x_headers_incparser() {}
			v1x_headers_incparser() {}
			v1x_headers_incparser(v1x_headers_incparser const &) = default;
			v1x_headers_incparser &operator=(v1x_headers_incparser const &) = default;
#ifndef CLANE_HAVE_NO_DEFAULT_MOVE
			v1x_headers_incparser(v1x_headers_incparser &&) = default;
			v1x_headers_incparser &operator=(v1x_headers_incparser &&) = default;
#endif

			void reset();
			size_t parse_some(char const *beg, char const *end);

			// accessors:
			header_map const &headers() const { return hdrs; }
			header_map &headers() { return hdrs; }
		};

		// Does not check against maximum length limit.
		class v1x_chunk_line_incparser: virtual public incparser {
			static int const max_nibs = 2 * sizeof(size_t);
			enum class state {
				digit,
				newline
			} cur_stat;
			size_t chunk_size_;
			int nibs;
		public:
			~v1x_chunk_line_incparser() {}
			v1x_chunk_line_incparser() {}
			v1x_chunk_line_incparser(v1x_chunk_line_incparser const &) = default;
			v1x_chunk_line_incparser &operator=(v1x_chunk_line_incparser const &) = default;
#ifndef CLANE_HAVE_NO_DEFAULT_MOVE
			v1x_chunk_line_incparser(v1x_chunk_line_incparser &&) = default;
			v1x_chunk_line_incparser &operator=(v1x_chunk_line_incparser &&) = default;
#endif
			// Note: Ignores length limit, uses fixed length limit based on
			// sizeof(size_t).
			void reset();
			size_t parse_some(char const *beg, char const *end);

			// accessors:
			size_t chunk_size() { return chunk_size_; }
		};

		// Does not check against maximum length limit.
		class v1x_body_incparser: virtual public incparser {
		public:
			enum length_type {
				fixed,   // preset length, in bytes
				chunked, // obeys rules for chunked transfer-encoding
				infinite // body ends when connection closes
			};
		private:
			enum class state {
				chunk_carriage_return,
				chunk_newline,
				chunk_line,
				body_data
			} cur_stat;
			length_type len_type;
			size_t rem; // bytes remaining if non-infinite, otherwise 0
			v1x_chunk_line_incparser chunk_pars;
			size_t offset_;
			size_t size_;
		public:
			~v1x_body_incparser() = default;
			v1x_body_incparser() = default;
			v1x_body_incparser(v1x_body_incparser const &) = default;
			v1x_body_incparser &operator=(v1x_body_incparser const &) = default;
#ifndef CLANE_HAVE_NO_DEFAULT_MOVE
			v1x_body_incparser(v1x_body_incparser &&) = default;
			v1x_body_incparser &operator=(v1x_body_incparser &&) = default;
#endif

			void reset(length_type len_type, size_t len);
			size_t parse_some(char const *beg, char const *end);

			// accessors:
			size_t offset() const { return offset_; }
			size_t size() const { return size_; }
		};

		class v1x_request_incparser: public v1x_request_line_incparser, public v1x_headers_incparser, public v1x_body_incparser {
			enum class state {
				request_line,
				headers,
				body,
				pre_trailers,
				trailers
			} cur_stat;
			header_map hdrs;
			size_t offset_;
			size_t size_;
			bool got_hdrs;
			bool chunked;
		public:
			~v1x_request_incparser() = default;
			v1x_request_incparser() = default;
			v1x_request_incparser(v1x_request_incparser const &) = default;
			v1x_request_incparser &operator=(v1x_request_incparser const &) = default;
#ifndef CLANE_HAVE_NO_DEFAULT_MOVE
			v1x_request_incparser(v1x_request_incparser &&) = default;
			v1x_request_incparser &operator=(v1x_request_incparser &&) = default;
#endif

			void reset();
			size_t parse_some(char const *beg, char const *end);

			// accessors:
			// Use base class accessors, too. Request line, headers, and body
			// offset and size are valid only after got_headers() returns true.
			// Trailers are valid only after parsing completes. The parse_some()
			// method guarantees to return at least once such that got_headers() is
			// true but before having parsed any of the body input data.
			bool got_headers() const { return got_hdrs; }
			header_map const &headers() const { return hdrs; }
			header_map &headers() { return hdrs; }
			size_t offset() const { return offset_; }
			size_t size() const { return size_; }
			header_map const &trailers() const { return v1x_headers_incparser::headers(); }
			header_map &trailers() { return v1x_headers_incparser::headers(); }
		};

		class v1x_response_incparser: public v1x_status_line_incparser, public v1x_headers_incparser,
		public v1x_body_incparser {
			enum class state {
				status_line,
				headers,
				body,
				pre_trailers,
				trailers
			} cur_stat;
			header_map hdrs;
			size_t offset_;
			size_t size_;
			bool got_hdrs;
			bool chunked;
		public:
			~v1x_response_incparser() = default;
			v1x_response_incparser() = default;
			v1x_response_incparser(v1x_response_incparser const &) = default;
			v1x_response_incparser &operator=(v1x_response_incparser const &) = default;
#ifndef CLANE_HAVE_NO_DEFAULT_MOVE
			v1x_response_incparser(v1x_response_incparser &&) = default;
			v1x_response_incparser &operator=(v1x_response_incparser &&) = default;
#endif

			void reset();
			size_t parse_some(char const *beg, char const *end);

			// accessors:
			// Use base class accessors, too. Status line, headers, and body
			// offset and size are valid only after got_headers() returns true.
			// Trailers are valid only after parsing completes. The parse_some()
			// method guarantees to return at least once such that got_headers() is
			// true but before having parsed any of the body input data.
			bool got_headers() const { return got_hdrs; }
			header_map const &headers() const { return hdrs; }
			header_map &headers() { return hdrs; }
			size_t offset() const { return offset_; }
			size_t size() const { return size_; }
			header_map const &trailers() const { return v1x_headers_incparser::headers(); }
			header_map &trailers() { return v1x_headers_incparser::headers(); }
		};

		class request {
		public:
			std::string method;
			uri::uri uri;
			int major_version;
			int minor_version;
			header_map headers;
			header_map trailers;
			std::istream body;
		private:
			std::unique_ptr<std::streambuf> sb;
		public:
			~request() = default;
			request(std::streambuf *sb): body{sb} {}
			request(std::unique_ptr<std::streambuf> &&sb): body{sb.get()}, sb{std::move(sb)} {}
			request(request const &) = delete;
			request &operator=(request const &) = delete;
#ifndef CLANE_HAVE_NO_DEFAULT_MOVE
			request(request &&) = default;
			request &operator=(request &&) = default;
#endif
		};

	}
}

#endif // #ifndef CLANE_HTTP_MESSAGE_HPP
