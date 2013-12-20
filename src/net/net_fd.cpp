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

		void close_fd(int fd) {
			int status = TEMP_FAILURE_RETRY(::close(fd));
			if (-1 == status) {
				std::ostringstream ess;
				ess << "file descriptor close error: " << errno_to_string(errno);
				throw std::runtime_error(ess.str());
			}
		}

		int fd_flags(int fd) {
			int flags = ::fcntl(fd, F_GETFL);
			if (-1 == flags) {
				std::ostringstream ess;
				ess << "error getting file descriptor flags: " << errno_to_string(errno);
				throw std::runtime_error(ess.str());
			}
			return flags;
		}

		void fd_flags(int fd, int flags) {
			int stat = ::fcntl(fd, F_SETFL, flags);
			if (-1 == stat) {
				std::ostringstream ess;
				ess << "error setting file descriptor flags: " << errno_to_string(errno);
				throw std::runtime_error(ess.str());
			};
		}

		void set_nonblocking(int fd) {
			int flags = fd_flags(fd);
			fd_flags(fd, flags | O_NONBLOCK);
		}

		void set_blocking(int fd) {
			int flags = fd_flags(fd);
			fd_flags(fd, flags & ~O_NONBLOCK);
		}

		void file_descriptor::close() {
			if (-1 == fd)
				return;
			struct reset_fd {
				file_descriptor *fd;
				reset_fd(file_descriptor *fd): fd(fd) {}
				~reset_fd() { fd->fd = -1; }
			} fd_resetter(this);
			close_fd(fd);
		}
	}
}

