// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// vim: set noet:

#ifndef CLANE_HTTP_PUB_HPP
#define CLANE_HTTP_PUB_HPP

/** @file
 *
 * @brief Hypertext Transfer Protocol */

#include "clane_ascii_pub.hpp"
#include "clane_base_pub.hpp"
#include "clane_net_pub.hpp"
#include "clane_sync_pub.hpp"
#include "clane_uri_pub.hpp"
#include <deque>
#include <istream>
#include <map>
#include <memory>
#include <thread>

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
			// Trailers are valid only after parsing completes.
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

		/** @brief Server-side class for writing an HTTP response message */
		class response_ostream: public std::ostream {
		public:

			/** @brief Status code to send
			 *
			 * @remark Applications may set the @ref status member to send an
			 * HTTP response message with a @link status_code status code
			 * @endlink other than the default value of `200 OK`. Setting the
			 * @ref status member has no effect after at least one byte of the
			 * body has been inserted into the response_ostream instance. */
			status_code &status;

			/** @brief Headers to send
			 *
			 * @remark Applications may insert one or more @link header HTTP
			 * headers @endlink into the @ref headers member to send those
			 * headers as part of the response message. Setting the @ref headers
			 * member has no effect after at least one byte of the body has been
			 * inserted into the response_ostream instance. */
			header_map &headers;

		public:
			virtual ~response_ostream() {}
			response_ostream(std::streambuf *sb, status_code &stat_code, header_map &hdrs);
			response_ostream(response_ostream const &) = delete;
			response_ostream &operator=(response_ostream const &) = delete;
#ifndef CLANE_HAVE_NO_DEFAULT_MOVE
			response_ostream(response_ostream &&) = default;
			response_ostream &operator=(response_ostream &&) = default;
#endif
		};

		inline response_ostream::response_ostream(std::streambuf *sb, status_code &stat_code, header_map &hdrs):
		 	std::ostream{sb}, status(stat_code), headers(hdrs) {}

		class server_streambuf: public std::streambuf {
			struct buffer {
				std::shared_ptr<char> p;
				size_t size;
			};
			net::socket &sock;
			int major_ver;
			int minor_ver;
		public:
			status_code out_stat_code;
			header_map out_hdrs;
		private:
			std::mutex in_mutex;
			bool in_end;
			std::deque<buffer> in_queue;
			std::condition_variable in_cond;
			std::mutex act_mutex;
			std::condition_variable act_cond;
			bool enabled;
			bool active;
			bool hdrs_written;
			bool chunked;
			char out_buf[4096];
		public:
			virtual ~server_streambuf();
			server_streambuf(net::socket &sock);
			server_streambuf(server_streambuf const &) = default;
			server_streambuf &operator=(server_streambuf const &) = default;
#ifndef CLANE_HAVE_NO_DEFAULT_MOVE
			server_streambuf(server_streambuf &&) = default;
			server_streambuf &operator=(server_streambuf &&) = default;
#endif
			void enable() { enabled = true; }
			void set_version(int major, int minor) { major_ver = major; minor_ver = minor; }
			void more_request_body(std::shared_ptr<char> const &p, size_t offset, size_t size);
			void end_request_body();
			void inactivate();
			void activate();
			bool headers_written() const { return hdrs_written; }
		protected:
			virtual int sync();
			virtual int_type underflow();
			virtual int_type overflow(int_type ch);
		private:
			int flush(bool end = false);
		};

		class server_context {
			sync::wait_group::reference wg_ref;
		public:
			server_streambuf sb;
			request req;
			response_ostream rs;
		private:
			std::mutex next_mutex;
			std::shared_ptr<server_context> next_ctx;
		public:
			~server_context();
			server_context(sync::wait_group::reference &&wg_ref, net::socket &sock): wg_ref{std::move(wg_ref)}, sb{sock},
				req{&sb}, rs{&sb, sb.out_stat_code, sb.out_hdrs} {}
			server_context(server_context const &) = delete;
			server_context(server_context &&) = delete;
			server_context &operator=(server_context const &) = delete;
			server_context &operator=(server_context &&) = delete;
			void set_next_context(std::shared_ptr<server_context> const &nc);
		private:
			void activate();
		};

		/** @brief HTTP server
		 *
		 * @tparam Handler An @ref http_request_handling_page "HTTP request handler"
		 *
		 * @remark A basic_server instance continually serves connections accepted
		 * from a set of listeners. The server hides the details of handling the
		 * network I/O and message parsing and calls a **root handler** once for
		 * each valid incoming request. The root handler generates the response for
		 * the request, possibly by calling another handler to do the work.
		 *
		 * @remark To set up a server, applications should:
		 *
		 * @remark <ol> <li>Construct a basic_server instance,
		 * <li>Set the server's root handler,
		 * <li>Add one or more listeners, and
		 * <li>Call the server's serve() method.
		 * </ol>
		 *
		 * @remark The server instance's serve() method blocks until the server
		 * terminates, either due to an unrecoverable error or via its terminate()
		 * method. As such, applications must call serve() within a dedicated
		 * thread. Internally, the server threading model is naive: it spawns a
		 * unique thread for each listener, for each connection, and for each
		 * incoming request. Future versions of @projectname may use fewer threads.
		 *
		 * @remark A basic_server is a template based on the handler type.
		 * @projectname also provides the non-templated @ref server type, which uses
		 * `std::function` for its request handler type.
		 *
		 * @sa @ref server
		 * @sa make_server()
		 *
		 */
		template <typename Handler> class basic_server {
			static size_t const default_max_header_size = 8 * 1024;
			std::deque<clane::net::socket> listeners;
			clane::net::event term_event;
			std::deque<std::thread> thrds;
			clane::sync::wait_group *conn_wg;
		public:

			/** @brief @ref http_request_handling_page "HTTP request handler" that
			 * receives all incoming requests
			 *
			 * @remark The server calls its **root handler** for each incoming HTTP
			 * request. The root handler may respond to the request directly or else
			 * dispatch the request to a sub-handler to do the work. Either way, when
			 * the root handler returns, the application has handled the request.
			 *
			 * @remark As a special case, if the root handler returns and (1) the
			 * response error status code @ref denotes_error "denotes an error" and
			 * (2) the response body is empty and not flushed then the server will add
			 * human-readable body content describing the error. This causes
			 * applications by default to send human-meaningful error responses---even
			 * errors originating from built-in handlers, such as basic_router---while
			 * allowing applications to customize all error responses. */
			Handler root_handler;

			size_t max_header_size;
			std::chrono::steady_clock::duration read_timeout;
			std::chrono::steady_clock::duration write_timeout;

		public:
			~basic_server() = default;
			basic_server();
			basic_server(Handler &&h);
			basic_server(basic_server const &) = delete;
			basic_server &operator=(basic_server const &) = delete;
#ifndef CLANE_HAVE_NO_DEFAULT_MOVE
			basic_server(basic_server &&) = default;
			basic_server &operator=(basic_server &&) = default;
#else
			basic_server(basic_server &&that) noexcept;
			basic_server &operator=(basic_server &&that) noexcept;
#endif

			void add_listener(char const *addr);
			void add_listener(std::string const &addr);
			void add_listener(net::socket &&lis);

			/** @brief Run the server
			 *
			 * @remark The serve() method runs the server in the current thread. The
			 * serve() method returns only after the server has terminated, either
			 * due to an unrecoverable error or via the terminate() method. If an
			 * unrecoverable error occurs then the serve() method throws an exception.
			 *
			 * @sa terminate() */
			void serve();

			/** @brief Stops the server
			 *
			 * @remark The terminate() method puts the server into a terminal state.
			 * If the server is currently running then it will begin shutting down. If
			 * instead the application starts the server in the future then the server
			 * will immediately terminate after it starts. The terminal state is
			 * permanent: a server cannot run twice.
			 *
			 * @remark When in a terminal state, the server doesn't accept new network
			 * connections and doesn't receive new HTTP requests on existing
			 * connections. If a connection has any outstanding requests—i.e., any
			 * requests currently being handled—then the server waits for all of those
			 * requests' root handler invocations to return before closing the
			 * connection. Once all connections have closed, the server frees
			 * remaining resources and terminates.
			 *
			 * @remark The terminate() method returns immediately, possibly before the
			 * server terminates. To determine when the server has terminated, an
			 * application should check for the return of its call to the serve()
			 * method.
			 *
			 * @sa serve() */
			void terminate();

		private:
			void listen_main(net::socket &&lis);
#ifndef CLANE_HAVE_STD_THREAD_MOVE_ARG
			void listen_main_move_wrapper(std::mutex &lis_mutex, std::condition_variable &lis_cond, net::socket **lis);
#endif
			void connection_main(net::socket &&conn);
#ifndef CLANE_HAVE_STD_THREAD_MOVE_ARG
			void connection_main_move_wrapper(std::mutex &conn_mutex, std::condition_variable &conn_cond, net::socket **conn);
#endif
		};

		/** @brief Specializes basic_server for a `std::function` request handler
		 *
		 * @remark The @ref server type may be easier to use than the basic_server
		 * template type but may incur additional runtime cost when otherwise not
		 * using a `std::function` handler type.
		 *
		 * @sa basic_server
		 * @sa make_server() */
		typedef basic_server<std::function<void(response_ostream &, request &)>> server;

		/** @brief Constructs and returns a basic_server instance
		 *
		 * @relatesalso basic_server
		 *
		 * @remark The make_server() function is a convenience function that allows
		 * an application to create a basic_server instance using template type
		 * deduction for the request handler type.
		 *
		 * @sa basic_server
		 * @sa server */
		template <typename Handler> basic_server<Handler> make_server(Handler &&h) {
			return basic_server<Handler>(std::forward<Handler>(h));
		}

		template <typename Handler> basic_server<Handler>::basic_server():
			max_header_size{default_max_header_size},
			read_timeout{0},
			write_timeout{0} {}

		template <typename Handler> basic_server<Handler>::basic_server(Handler &&h):
			root_handler{std::forward<Handler>(h)},
			max_header_size{default_max_header_size},
			read_timeout{0},
			write_timeout{0} {}

#ifdef CLANE_HAVE_NO_DEFAULT_MOVE

		template <typename Handler> basic_server<Handler>::basic_server(basic_server &&that) noexcept:
			root_handler{std::move(that.root_handler)},
			max_header_size{std::move(that.max_header_size)},
			read_timeout{std::move(that.read_timeout)},
			write_timeout{std::move(that.write_timeout)} {}

		template <typename Handler> basic_server<Handler> &basic_server<Handler>::operator=(basic_server &&that) noexcept {	
			root_handler = std::move(that.root_handler);
			max_header_size = std::move(that.max_header_size);
			read_timeout = std::move(that.read_timeout);
			write_timeout = std::move(that.write_timeout);
			return *this;
		}

#endif

		template <typename Handler> void basic_server<Handler>::add_listener(char const *addr) {
			listeners.push_back(listen(&net::tcp, addr));
			listeners.back().set_nonblocking();
		}

		template <typename Handler> void basic_server<Handler>::add_listener(std::string const &addr) {
			listeners.push_back(listen(&net::tcp, addr));
			listeners.back().set_nonblocking();
		}

		template <typename Handler> void basic_server<Handler>::add_listener(net::socket &&lis) {
			lis.set_nonblocking();
			listeners.push_back(std::move(lis));
		}

		template <typename Handler> void basic_server<Handler>::serve() {

			sync::wait_group wg; // for waiting on connections to stop
			conn_wg = &wg;

			// unique thread for each listener:

			while (!listeners.empty()) {
#ifdef CLANE_HAVE_STD_THREAD_MOVE_ARG
				thrds.push_back(std::thread(&basic_server::listen_main, this, std::move(listeners.front())));
#else
				// If std::thread doesn't support rvalue reference arguments then move
				// the socket inside the thread.
				net::socket lis = std::move(listeners.front());
				net::socket *plis = &lis;
				std::mutex lis_mutex;
				std::condition_variable lis_cond;
				std::unique_lock<std::mutex> lis_lock(lis_mutex);
				thrds.push_back(std::thread(&basic_server::listen_main_move_wrapper, this, std::ref(lis_mutex), std::ref(lis_cond), &plis));
				while (plis)
					lis_cond.wait(lis_lock);
#endif
				listeners.pop_front();
			}

			// wait for the termination signal:
			net::poller poller;
			poller.add(term_event, poller.in);
			poller.poll();

			// wait for all listeners to stop:
			for (auto i = thrds.begin(); i != thrds.end(); ++i)
				i->join();
			thrds.clear();

			// The connection wait group will cause this thread to block until
			// all connection threads have stopped.
		}

		template <typename Handler> void basic_server<Handler>::terminate() {
			term_event.signal();
		}

#ifndef CLANE_HAVE_STD_THREAD_MOVE_ARG
		template <typename Handler> void basic_server<Handler>::listen_main_move_wrapper(std::mutex &lis_mutex,
		std::condition_variable &lis_cond, net::socket **lis) {
			std::unique_lock<std::mutex> lis_lock(lis_mutex);
			net::socket mlis = std::move(**lis);
			*lis = nullptr;
			lis_cond.notify_one();
			lis_lock.unlock();
			listen_main(std::move(mlis));
		}
#endif

		template <typename Handler> void basic_server<Handler>::listen_main(net::socket &&lis) {

			net::poller poller;
			size_t const iterm = poller.add(term_event, poller.in);
			poller.add(lis, poller.in);

			// accept incoming connections, and launch a unique thread for each new
			// connection:
			auto poll_res = poller.poll();
			while (poll_res.index != iterm) {
				std::error_code e;
				net::socket conn = lis.accept(e);
				if (e)
					continue; // ignore error
#ifdef CLANE_HAVE_STD_THREAD_MOVE_ARG
				std::thread conn_thrd(&basic_server::connection_main, this, std::move(conn));
#else
				// If std::thread doesn't support rvalue reference arguments then move
				// the socket inside the thread.
				net::socket *pconn = &conn;
				std::mutex conn_mutex;
				std::condition_variable conn_cond;
				std::unique_lock<std::mutex> conn_lock(conn_mutex);
				std::thread conn_thrd(&basic_server::connection_main_move_wrapper, this, std::ref(conn_mutex), std::ref(conn_cond), &pconn);
				while (pconn)
					conn_cond.wait(conn_lock);
#endif
				conn_thrd.detach();
				poll_res = poller.poll();
			}
		}

#ifndef CLANE_HAVE_STD_THREAD_MOVE_ARG
		template <typename Handler> void basic_server<Handler>::connection_main_move_wrapper(std::mutex &conn_mutex,
		std::condition_variable &conn_cond, net::socket **conn) {
			std::unique_lock<std::mutex> conn_lock(conn_mutex);
			net::socket mconn = std::move(**conn);
			*conn = nullptr;
			conn_cond.notify_one();
			conn_lock.unlock();
			connection_main(std::move(mconn));
		}
#endif

		template <typename Handler> void handler_main(Handler &h, std::shared_ptr<server_context> ctx) {

			// root handler:
			h(ctx->rs, ctx->req);

			// Special case: If the root handler responds with (1) an error status
			// code (4xx or 5xx) and (2) an empty and non-flushed body then fill out
			// the body with the status code reason phrase. This causes error
			// responses to be human-meaningful by default but still allows
			// applications to customize the error response page--even for error
			// responses generated by built-in handlers such as the basic_router.

			if (!ctx->sb.headers_written() && denotes_error(ctx->rs.status)) {
				auto r = ctx->rs.headers.equal_range("content-type");
				ctx->rs.headers.erase(r.first, r.second);
#ifdef CLANE_HAVE_STD_MULTIMAP_EMPLACE
				ctx->rs.headers.emplace("content-type", "text/plain");
#else
				ctx->rs.headers.insert(http::header("content-type", "text/plain"));
#endif
				ctx->rs << what(ctx->rs.status) << '\n';
			}
		}

		template <typename Handler> void basic_server<Handler>::connection_main(net::socket &&conn) {

			auto my_ref = conn_wg->new_reference();
			sync::wait_group req_wg; // for waiting on request-handler threads to complete

			conn.set_nonblocking();

			// input buffer:
			static size_t const incap = 4096;
			std::shared_ptr<char> inbuf;
			size_t inoff = incap;
			size_t insiz;

			// request-handler context:
			auto cur_ctx = std::make_shared<server_context>(req_wg.new_reference(), conn);

			// parsing:
			v1x_request_incparser pars;
			pars.reset();
			pars.set_length_limit(max_header_size);
			bool got_hdrs = false;

			// I/O multiplexing:
			std::chrono::steady_clock::time_point to;
			if (std::chrono::steady_clock::duration::zero() != read_timeout)
				to = std::chrono::steady_clock::now() + read_timeout;
			net::poller poller;
			size_t const iterm = poller.add(term_event, poller.in);
			poller.add(conn, poller.in);

			// consume incoming data from the connection:
			while (true) {

				// wait for event: data, termination, or timeout
				auto poll_res = std::chrono::steady_clock::duration::zero() == to.time_since_epoch() ? poller.poll() : poller.poll(to);
				if (!poll_res.index) {
					// FIXME: timeout
					goto done;
				}
				if (poll_res.index == iterm) {
					// FIXME: termination
					goto done;
				}

				// reallocate input buffer if full:
				if (inoff == incap) {
					inbuf = std::unique_ptr<char, std::default_delete<char[]>>(new char[incap]);
					inoff = insiz = 0;
				}

				// receive:
				{
					std::error_code e;
					size_t xstat = conn.recv(reinterpret_cast<char *>(inbuf.get()) + inoff, incap - inoff, e);
					if (e == std::errc::operation_would_block || e == std::errc::resource_unavailable_try_again)
						continue; // go back to waiting
					if (e) {
						// FIXME: connection error
						goto done;
					}
					if (!xstat) {
						// FIXME: connection FIN
						goto done;
					}
					insiz = xstat;
				}

				// process the received data:
				while (insiz) {

					// parse:
					size_t pstat = pars.parse_some(inbuf.get()+inoff, inbuf.get()+inoff+insiz);
					if (pars.error == pstat) {
						// FIXME: error
						goto done;
					}

					// FIXME: check HTTP version

					if (pars.got_headers()) {

						// got new request?
						if (!got_hdrs) {

							// set up request object:
							got_hdrs = true;
							cur_ctx->sb.enable();
							cur_ctx->req.method = std::move(pars.method());
							cur_ctx->req.uri = std::move(pars.uri());
							cur_ctx->req.major_version = pars.major_version();
							cur_ctx->req.minor_version = pars.minor_version();
							cur_ctx->sb.set_version(cur_ctx->req.major_version, cur_ctx->req.minor_version);
							cur_ctx->req.headers = std::move(pars.headers());

							// start request handler:
							std::thread(&handler_main<Handler>, std::ref(root_handler), cur_ctx).detach();
						}

						// feed body data to request object:
						cur_ctx->sb.more_request_body(inbuf, inoff+pars.offset(), pars.size());
					}

					inoff += pstat;
					insiz -= pstat;

					if (pars)
						continue;

					// request is complete:
					cur_ctx->req.trailers = std::move(pars.trailers());
					cur_ctx->sb.end_request_body();

					// prepare for next request:
					{
						auto next_ctx = std::make_shared<server_context>(req_wg.new_reference(), conn);
						cur_ctx->set_next_context(next_ctx); // set up pipeline dependency
						cur_ctx = std::move(next_ctx);
						pars.reset();
						got_hdrs = false;
					}
				}
			}
done: // connection is finished, regardless whether graceful or not
			{}
		}

	}
}

#endif // #ifndef CLANE_HTTP_PUB_HPP
