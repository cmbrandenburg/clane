// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// vim: set noet:

#ifndef CLANE_HTTP_MESSAGE_HPP
#define CLANE_HTTP_MESSAGE_HPP

/** @file */

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
	}
}

#endif // #ifndef CLANE_HTTP_MESSAGE_HPP
