// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// vim: set noet:

/** @file */

#ifndef CLANE_HTTP_SERVER_HPP
#define CLANE_HTTP_SERVER_HPP

#include <string>

namespace clane {
	namespace http {

		/** @brief Parses a line, handling both LF and CRLF line endings
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
		 * @return On success, returns the number of bytes parsed. Otherwise, if the
		 * result string would exceed `max`, then returns `std::string::npos`. */
		std::size_t parse_line(char const *p, std::size_t n, std::size_t max, std::string &oline, bool &ocomplete);

		template <typename InputIter> bool is_valid_method(InputIter first, InputIter last) {
			// TODO: implement
			return false;
		}

		template <typename InputIter> bool is_valid_http_version(InputIter first, InputIter last) {
			// TODO: implement
			return false;
		}
	}
}

#endif // #ifndef CLANE_HTTP_SERVER_HPP
