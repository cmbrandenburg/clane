// vim: set noet:

/** \file */

#include "net_error.h"
#include "net_fd.h"
#include <errno.h>
#include <fcntl.h>
#include <sstream>
#include <stdexcept>
#include <unistd.h>

namespace clane {
	namespace net {

		void file_descriptor::close() {
			if (-1 == fd)
				return;
			int status;
			while (-1 == (status = ::close(fd)) && EINTR == errno);
			if (-1 == status) {
				std::ostringstream ss;
				ss << "error closing file descriptor: " << safe_strerror(errno);
				throw std::runtime_error(ss.str());
			}
			fd = -1;
		}

		void file_descriptor::set_nonblocking() const {
			if (-1 == fd)
				return;
			int flags = ::fcntl(fd, F_GETFL);
			if (-1 == flags) {
				std::ostringstream ss;
				ss << "error setting nonblocking mode: error getting file descriptor flags: " << safe_strerror(errno);
				throw std::runtime_error(ss.str());
			}
			flags |= O_NONBLOCK;
			int status = ::fcntl(fd, F_SETFL, flags);
			if (-1 == status) {
				std::ostringstream ss;
				ss << "error setting nonblocking mode: error setting file descriptor flags: " << safe_strerror(errno);
				throw std::runtime_error(ss.str());
			}
		}
	}
}

