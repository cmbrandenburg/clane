// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// vim: set noet:

#ifndef CLANE_HTTP_MESSAGE_HXX
#define CLANE_HTTP_MESSAGE_HXX

/** @file */

#include "clane/clane_http_message.hpp"

namespace clane {
	namespace http {

		/** Returns whether a string is a valid HTTP method */
		bool is_method(char const *beg, char const *end);

		/** Returns whether a string is valid HTTP _TEXT_ (as according to RFC 2616) */
		bool is_text(char const *beg, char const *end);

		/** Returns whether a string is a valid HTTP version field */
		bool parse_http_version(char const *beg, char const *end, unsigned &omajor, unsigned &ominor);

		/** Parser for HTTP 1.x-style headers list */
		class v1x_headers_parser {
			typedef http::status_code status_code_type;
		public:
			static constexpr std::size_t cap = 16384; // maximum size of all headers, in bytes
		private:
			bool             m_bad{};
			bool             m_done{};
			status_code_type m_stat_code;
			std::size_t      m_size{};   // number of input bytes parsed
			std::string      m_cur_line;
			header           m_cur_hdr;
			header_map       m_hdrs;
		public:
			bool okay() const { return !m_bad; }
			bool done() const { return m_done; }
			status_code_type status_code() const { return m_stat_code; }
			void reset();
			std::size_t parse(char const *p, std::size_t n);
			header_map &headers() { return m_hdrs; }
		};

	}
}

#endif // #ifndef CLANE_HTTP_MESSAGE_HXX
