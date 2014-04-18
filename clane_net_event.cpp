// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// vim: set noet:

/** @file */

#include "clane_net_error.hpp"
#include "clane_net_event.hpp"
#include <sys/eventfd.h>
#include <unistd.h>

namespace clane {
	namespace net {

		event::event() {
			fd = ::eventfd(0, EFD_NONBLOCK);
			if (-1 == fd)
				throw std::system_error(errno, os_category(), "create event");
		}

		void event::signal() {
			uint64_t n = 1;
			ssize_t stat = ::write(fd, &n, sizeof(n));
			if (-1 == stat && EAGAIN != errno)
				throw std::system_error(errno, os_category(), "signal event");
		}

		void event::reset() {
			uint64_t n;
			ssize_t stat = ::read(fd, &n, sizeof(n));
			if (-1 == stat && EAGAIN != errno)
				throw std::system_error(errno, os_category(), "reset event");
		}
	}
}

