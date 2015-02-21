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

		/** Returns whether a string is a valid HTTP version field */
		bool parse_http_version(char const *beg, char const *end, unsigned &omajor, unsigned &ominor);

		/** Encapsulates the result of one iteration of interruptible parsing
		 *
		 * @remark A parse_result signifies one of three categories of parser
		 * results:
		 * -# _Unrecoverable error_, in which case the connection should be
		 * closed,
		 * -# _Complete_, in which case the headers are available,
		 * -# _Incomplete_, in which case the headers are _not_ available.
		 *
		 * @remark If an error occurs then there's an associated HTTP status
		 * code signifying the type of error. Otherwise, there's an associated
		 * size quantifying how many bytes of input the parser consumed. */
		struct parse_result {
			enum {
				error,
				done,
				not_done
			} category;
			std::size_t size;
			status_code stat_code;
		};

		/** Parser for HTTP 1.x-style headers list */
		class v1x_headers_parser {
		public:
			static constexpr std::size_t cap = 16384; // maximum size of all headers, in bytes
		private:
			std::size_t m_size;     // number of input bytes parsed
			std::string m_cur_line;
			header      m_cur_hdr;
			header_map  m_hdrs;
		public:
			void reset();
			parse_result parse(char const *p, std::size_t n);
			header_map &headers() { return m_hdrs; }
		};

	}
}

#endif // #ifndef CLANE_HTTP_MESSAGE_HXX
