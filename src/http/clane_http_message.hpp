// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// vim: set noet:

#ifndef CLANE_HTTP_MESSAGE_HPP
#define CLANE_HTTP_MESSAGE_HPP

/** @file */

#include "clane/clane_http.hpp"
#include <string>

namespace clane {
	namespace http {

		/** Returns whether a string is a valid HTTP method */
		bool is_method(char const *beg, char const *end);

		/** Returns whether a string is a valid HTTP version field */
		bool parse_http_version(char const *beg, char const *end, unsigned &omajor, unsigned &ominor);

#if 0 // FIXME
		/** Parses a line, handling both LF and CRLF line endings
		 *
		 * @param p Input buffer to parse.
		 *
		 * @param n Number of bytes to parse in the input buffer.
		 *
		 * @param max Maximum number of bytes to allow in the result string.
		 *
		 * @param oline The result string, excluding any line-ending characters.
		 *
		 * @param ocomplete Assigned true if and only if a full string is parsed.
		 * Otherwise, assigned false.
		 *
		 * @return On success, the parse_line() function returns the number of bytes
		 * parsed. Otherwise, if the result string would exceed `max`, then the
		 * parse_line() function returns zero. */
		std::size_t parse_line(char const *p, std::size_t n, std::size_t max, std::string &oline, bool &ocomplete);

#endif // #if 0


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
		class parse_result {
		public:
			static struct complete_type {} complete;
			static struct incomplete_type {} incomplete;
		private:
			typedef http::status_code status_code_type;
			bool m_error{};
			bool m_done{};
			std::size_t m_size;
			status_code_type m_stat_code;
		public:
			parse_result(status_code_type stat_code): m_error{true}, m_stat_code{stat_code} {}
			parse_result(complete_type const &x, std::size_t size): m_done{true}, m_size{size} {}
			parse_result(incomplete_type const &x, std::size_t size): m_size{size} {}
			bool error() const { return m_error; }
			bool done() const { return m_done; }
			std::size_t size() const { return m_size; }
			status_code_type status_code() const { return m_stat_code; }
		};

		/** Parser for HTTP 1.x-style headers list */
		class v1x_headers_parser {
			enum phase {
				begin_name,
			} m_phase;
			header_map m_hdrs;
			header m_cur; // current header
		public:
			v1x_headers_parser() { reset(); }
			void reset();
			parse_result parse(char const *p, std::size_t n);
			header_map &headers() { return m_hdrs; }
		};

	}
}

#endif // #ifndef CLANE_HTTP_MESSAGE_HPP
