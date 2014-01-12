// vim: set noet:

#include "clane_event.hpp"
#include <sstream>
#include <stdexcept>
#include <sys/eventfd.h>
#include <unistd.h>

namespace clane {
	namespace net {

		event::event() {
			fd = ::eventfd(0, EFD_NONBLOCK);
			if (-1 == fd) {
				std::ostringstream ess;
				ess << "create event: " << posix::errno_to_string(errno);
				throw std::runtime_error(ess.str());
			}
		}

		void event::signal() {
			uint64_t n = 1;
			ssize_t stat = ::write(fd, &n, sizeof(n));
			if (-1 == stat && EAGAIN != errno) {
				std::ostringstream ess;
				ess << "signal event: " << posix::errno_to_string(errno);
				throw std::runtime_error(ess.str());
			}
		}

		void event::reset() {
			uint64_t n;
			ssize_t stat = ::read(fd, &n, sizeof(n));
			if (-1 == stat && EAGAIN != errno) {
				std::ostringstream ess;
				ess << "reset event: " << posix::errno_to_string(errno);
				throw std::runtime_error(ess.str());
			}
		}
	}
}

