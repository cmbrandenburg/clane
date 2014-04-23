// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// vim: set noet:

#ifndef CLANE_ASCII_HPP
#define CLANE_ASCII_HPP

/** @file */

#include "clane_base.hpp"
#include <algorithm>
#include <cctype>
#include <string>

namespace clane {

	namespace ascii {

		/** @brief Case-insensitive string comparison */
		template <typename InputIter> int icase_compare(InputIter abeg, InputIter aend, InputIter bbeg, InputIter bend) {
			size_t const alen = aend - abeg;
			size_t const blen = bend - bbeg;
			size_t n = std::min(alen, blen);
			InputIter a = abeg;
			InputIter b = bbeg;
			for (size_t i = 0; i < n; ++i) {
				char la = std::tolower(*a);
				char lb = std::tolower(*b);
				if (la < lb)
					return -1;
				if (la > lb)
					return 1;
				++a;
				++b;
			}
			if (n < alen)
				return 1;
			if (n < blen)
				return -1;
			return 0;
		}

		/** @brief Case-insensitive string comparison */
		inline int icase_compare(std::string const &a, std::string const &b) {
			return icase_compare(a.begin(), a.end(), b.begin(), b.end());
		}

	}

}

#endif // #ifndef CLANE_ASCII_HPP
