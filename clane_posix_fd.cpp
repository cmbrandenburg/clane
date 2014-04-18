// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// vim: set noet:

/** @file */

#include "clane_posix_fd.hpp"
#include <fcntl.h>
#include <system_error>
#include <unistd.h>

namespace clane {
	namespace posix {

		static int fd_flags(int fd) {
			int flags = ::fcntl(fd, F_GETFL);
			if (-1 == flags)
				throw std::system_error(errno, std::generic_category(), "fcntl: get file status flags");
			return flags;
		}

		static void fd_flags(int fd, int flags) {
			int stat = ::fcntl(fd, F_SETFL, flags);
			if (-1 == stat)
				throw std::system_error(errno, std::generic_category(), "fcntl: set file status flags");
		}

		void close_fd(int fd) {
			int stat = TEMP_FAILURE_RETRY(::close(fd));
			if (-1 == stat)
				throw std::system_error(errno, std::generic_category(), "close");
		}

		void set_nonblocking(int fd) {
			int flags = fd_flags(fd);
			fd_flags(fd, flags | O_NONBLOCK);
		}
	}
}

