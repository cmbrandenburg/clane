// vim: set noet:

#include "../../clane_check.hpp"
#include "../clane_posix_fd.hpp"
#include <cerrno>
#include <fcntl.h>
#include <system_error>
#include <unistd.h>

using namespace clane;

static bool is_file_descriptor_open(int fd) {
	int status = fcntl(fd, F_GETFD);
	if (-1 == status && EBADF == errno)
		return false;
	if (-1 == status)
		throw std::system_error(errno, std::generic_category(), "fcntl: get file status flags");
	return true;
}

static int open_file_descriptor() {
	int fds[2];
	int stat = pipe(fds);
	if (-1 == stat)
		throw std::system_error(errno, std::generic_category(), "fcntl: set file status flags");
	posix::close_fd(fds[1]);
	return fds[0];
};

int main() {

	// default constructor: */
	{
		posix::unique_fd fd;
	}

	// constructor to transfer ownership of raw int: */
	{
		posix::unique_fd fd(open_file_descriptor());
		check(is_file_descriptor_open(fd));
	}

	// move constructor:
	{
		posix::unique_fd fd1(open_file_descriptor());
		posix::unique_fd fd2 = std::move(fd1);
		check(!is_file_descriptor_open(fd1));
		check(is_file_descriptor_open(fd2));
	}

	// move assignment:
	{
		posix::unique_fd fd1(open_file_descriptor());
		posix::unique_fd fd2;
		fd2 = std::move(fd1);
		check(!is_file_descriptor_open(fd1));
		check(is_file_descriptor_open(fd2));
	}

	// assignment to transfer ownership of raw int: */
	{
		posix::unique_fd fd;
		int sys_fd = open_file_descriptor();
		fd = sys_fd;
		check(is_file_descriptor_open(fd));
	}

	// explicit close:
	{
		int sys_fd = open_file_descriptor();
		posix::unique_fd fd(sys_fd);
		fd.close();
		check(!is_file_descriptor_open(sys_fd));
	}

	// release:
	{
		posix::unique_fd fd(open_file_descriptor());
		int sys_fd = fd.release();
		check(-1 == int{fd});
		fd = sys_fd;
	}

	return 0;
}

