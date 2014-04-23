// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// vim: set noet:

#ifndef CLANE_REGEX_HPP
#define CLANE_REGEX_HPP

/** @file */

#include "clane_base.hpp"
#include <boost/regex.hpp>
#include <cassert>

namespace clane {
	namespace regex {

		typedef int options_type;

		/** @brief Regular expression syntax options */
		namespace options {

			enum {
				perl,
				literal,
				normal = perl
			};
		}

		inline boost::regex_constants::syntax_option_type boost_regex_option(options_type t) {
			switch (t) {
				case options::perl: return boost::regex::perl;
				case options::literal: return boost::regex::literal;
				default: assert(false);
			}
		}

	}
}

#endif // #ifndef CLANE_REGEX_HPP
