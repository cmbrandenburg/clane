// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// vim: set noet:

#ifndef CLANE__HTTP_MESSAGE_HPP
#define CLANE__HTTP_MESSAGE_HPP

/** @file */

#include "../clane_base.hpp"
#include "../ascii/clane_ascii.hpp"
#include "../uri/clane_uri.hpp"
#include "../../include/clane_http_message.hpp"

namespace clane {
	namespace http {

		/** @brief Modifies a string in place to conform to HTTP 1.x header name
		 * capitalization standard—e.g., <code>"Content-Length"</code>. */
		void canonize_1x_header_name(char *beg, char *end);

		inline std::string canonize_1x_header_name(std::string s) {
			canonize_1x_header_name(&s[0], &s[s.size()]);
			return s;
		}

		class response {
		public:
			int major_version;
			int minor_version;
			status_code status;
			std::string reason;
			header_map headers;
			header_map trailers;
			std::istream body;
		private:
			std::unique_ptr<std::streambuf> sb;
		public:
			~response() = default;
			response(std::streambuf *sb): body{sb} {}
			response(std::unique_ptr<std::streambuf> &&sb): body{sb.get()}, sb{std::move(sb)} {}
			response(response const &) = delete;
			response &operator=(response const &) = delete;
#ifndef CLANE_HAVE_NO_DEFAULT_MOVE
			response(response &&) = default;
			response &operator=(response &&) = default;
#endif
		};

	}
}

#endif // #ifndef CLANE__HTTP_MESSAGE_HPP
