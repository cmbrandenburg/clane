// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// vim: set noet:

#ifndef CLANE__ASCII_HPP
#define CLANE__ASCII_HPP

/** @file */

#include "../clane_base.hpp"
#include "../../include/clane_ascii.hpp"
#include <algorithm>
#include <cassert>
#include <string>

namespace clane {

	namespace ascii {

		/** @brief Search a string for a newline
		 *
		 * @remark The find_newline() function searches a given memory block for the
		 * first newline. If a carriage return and newline pair is found
		 * (<code>"\r\n"</code>) then this returns a pointer to the carriage return
		 * character. Else, if a newline is found then this returns a pointer to the
		 * newline character. Else, if a carriage return is found at the last byte
		 * of the block then this returns a pointer to that carriage return. Else,
		 * this returns a pointer to the first byte after the memory block.
		 *
		 * @remark In other words, the result of this function is to return a
		 * pointer to the first "unreadable" byte in or out of the block, where
		 * readable characters are characters in the current line, excluding
		 * <code>"\r\n"</code> and <code>"\n"</code>. Note that a carriage return
		 * followed by a character other than a newline is considered readable. */
		inline char const *find_newline(char const *beg, char const *end) {
			assert(beg <= end);
			char const *newline = std::find(beg, end, '\n');
			if (newline > beg && *(newline-1) == '\r')
				return newline - 1; // carriage return before newline
			if (newline != end)
				return newline; // newline
			if (beg < end && *(end-1) == '\n')
				return end - 1; // carriage return at end of block
			return end;
		}

		/** @brief Modifies a string by removing any whitespace at its end */
		inline void rtrim(std::string &s) {
			s.erase(std::find_if_not(s.rbegin(), s.rend(), isspace).base(), s.end());
		}

		inline char const *skip_whitespace(char const *beg, char const *end) {
			char const *p = beg;
			while (p < end && std::isspace(*p)) {
				++p;
			}
			return p;
		}

		/** @brief Converts a hexadecimal character to its integer value (e.g., '5'
		 * → 5 and 'C' → 12). */
		inline int hexch_to_int(char c) {
			assert(std::isxdigit(c));
			if (std::isdigit(c))
				return c - '0';
			if (std::islower(c))
				return c - 'a' + 10;
			return c - 'A' + 10;
		}

		/** @brief Converts an integer value between 0 and 15 to its equivalent
		 * hexadecimal character (e.g., 5 → '5' and 12 → 'C') */
		inline char int_to_hexch(int n) {
			assert(0 <= n && n < 16);
			if (n < 10)
				return '0' + n;
			return 'A' + (n - 10);
		}

	}
}

#endif // #ifndef CLANE__ASCII_HPP
