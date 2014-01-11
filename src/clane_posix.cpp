// vim: set noet:

#include "clane_posix.hpp"
#include <fcntl.h>
#include <sstream>
#include <stdexcept>
#include <string.h>
#include <unistd.h>

namespace clane {
	namespace posix {

		int fd_flags(int fd) {
			int flags = ::fcntl(fd, F_GETFL);
			if (-1 == flags) {
				std::ostringstream ess;
				ess << "get file descriptor flags: " << errno_to_string(errno);
				throw std::runtime_error(ess.str());
			}
			return flags;
		}

		void fd_flags(int fd, int flags) {
			int stat = ::fcntl(fd, F_SETFL, flags);
			if (-1 == stat) {
				std::ostringstream ess;
				ess << "set file descriptor flags: " << errno_to_string(errno);
				throw std::runtime_error(ess.str());
			};
		}

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

		void set_nonblocking(int fd) {
			int flags = fd_flags(fd);
			fd_flags(fd, flags | O_NONBLOCK);
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

