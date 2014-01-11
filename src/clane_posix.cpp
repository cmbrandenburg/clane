// vim: set noet:

#include "clane_posix.hpp"
#include <sstream>
#include <stdexcept>
#include <string.h>
#include <unistd.h>

namespace clane {
	namespace posix {

		std::string errno_to_string(int n) {
			char buf[256];
			return strerror_r(n, buf, sizeof(buf));
		}

		void close_fd(int fd) {
			int status = TEMP_FAILURE_RETRY(::close(fd));
			if (-1 == status) {
				std::ostringstream ess;
				ess << "close file descriptor: " << errno_to_string(errno);
				throw std::runtime_error(ess.str());
			}
		}

		void file_descriptor::close() {
			if (-1 == fd)
				return;
			int n = fd;
			fd = -1;
			close_fd(n);
		}
	}
}

