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
