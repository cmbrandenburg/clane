// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// vim: set noet:

#ifndef CLANE_POSIX_FD_HPP
#define CLANE_POSIX_FD_HPP

/** @file */

#include "clane_base.hpp"
#include "include/clane_posix_pub.hpp"

namespace clane {
	namespace posix {

		/** @brief Sets a POSIX-style file descriptor to nonblocking mode */
		void set_nonblocking(int fd);

	}
}

#endif // #ifndef CLANE_POSIX_FD_HPP
