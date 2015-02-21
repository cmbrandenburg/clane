// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// vim: set noet:

#ifndef CLANE_ASCII_HXX
#define CLANE_ASCII_HXX

/** @file */

#include <cassert>
#include <cstring>

namespace clane {
	namespace ascii {

		/** Converts a hexadecimal character to its integer value (e.g., '5' → 5 and
		 * 'C' → 12). */
		inline int hexch_to_int(char c) {
			if ('0' <= c && c <= '9')
				return c - '0';
			if ('a' <= c && c <= 'f')
				return c - 'a' + 10;
			assert('A' <= c && c <= 'F');
			return c - 'A' + 10;
		}

		/** Converts an integer value between 0 and 15 to its equivalent
		 * hexadecimal character (e.g., 5 → '5' and 12 → 'C') */
		inline char int_to_hexch(int n) {
			assert(0 <= n && n < 16);
			if (n < 10)
				return '0' + n;
			return 'A' + (n - 10);
		}

		/** Returns whether a string begins with an exact substring */
		inline bool has_prefix(char const *beg, char const *end, char const *key) {
			assert(beg <= end);
			return static_cast<size_t>(end-beg) >= std::strlen(key) && !std::strncmp(beg, key, std::strlen(key));
		}

	}
}

#endif // #ifndef CLANE_ASCII_HXX
