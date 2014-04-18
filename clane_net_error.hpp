// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// vim: set noet:

#ifndef CLANE_NET_ERROR_HPP
#define CLANE_NET_ERROR_HPP

/** @file */

#include "clane_base.hpp"
#include <system_error>

namespace clane {

	// FIXME: Should this go in a separate 'sys' namespace?
	namespace net {

		// In C++11, std::system_category() error codes whose value semantically
		// maps to a POSIX errno value are supposed to be equivalent to
		// std::generic_category() error codes with the mapped errno value. However,
		// gcc and libstdc++ don't do this—not yet. So os_category() is a
		// placeholder until when std::system_category() does the right thing.
		inline std::error_category const &os_category() {
			return std::generic_category();
		}
	}
}

#endif // #ifndef CLANE_NET_ERROR_HPP
