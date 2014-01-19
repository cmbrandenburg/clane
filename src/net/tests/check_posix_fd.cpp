// vim: set noet:

#include "../../check/check.h"
#include "../net_posix.hpp"
#include <fcntl.h>
#include <sstream>
#include <stdexcept>
#include <unistd.h>

using namespace clane;

bool is_file_descriptor_open(int fd);
int open_file_descriptor();

bool is_file_descriptor_open(int fd) {
	int status = fcntl(fd, F_GETFD);
	if (-1 == status && EBADF == errno)
		return false;
	if (-1 == status) {
		std::ostringstream ess;
		ess << "get file descriptor flags: " << posix::errno_to_string(errno);
		throw std::runtime_error(ess.str());
	}
	return true;
}

int open_file_descriptor() {
	int fds[2];
	int stat = pipe(fds);
	if (-1 == stat) {
		std::ostringstream ess;
		ess << "open event file descriptor: " << posix::errno_to_string(errno);
		throw std::runtime_error(ess.str());
	}
	posix::close_fd(fds[1]);
	return fds[0];
};

int main() {

	// default constructor: */
	{
		posix::file_descriptor fd;
	}

	// constructor to transfer ownership of raw int: */
	{
		posix::file_descriptor fd(open_file_descriptor());
		check(is_file_descriptor_open(fd));
	}

	// move constructor:
	{
		posix::file_descriptor fd1(open_file_descriptor());
		posix::file_descriptor fd2 = std::move(fd1);
		check(!is_file_descriptor_open(fd1));
		check(is_file_descriptor_open(fd2));
	}

	// move assignment:
	{
		posix::file_descriptor fd1(open_file_descriptor());
		posix::file_descriptor fd2;
		fd2 = std::move(fd1);
		check(!is_file_descriptor_open(fd1));
		check(is_file_descriptor_open(fd2));
	}

	// assignment to transfer ownership of raw int: */
	{
		posix::file_descriptor fd;
		int sys_fd = open_file_descriptor();
		fd = sys_fd;
		check(is_file_descriptor_open(fd));
	}

	// explicit close:
	{
		int sys_fd = open_file_descriptor();
		posix::file_descriptor fd(sys_fd);
		fd.close();
		check(!is_file_descriptor_open(sys_fd));
	}

	// release:
	{
		posix::file_descriptor fd(open_file_descriptor());
		int sys_fd = fd.release();
		check(-1 == int{fd});
		fd = sys_fd;
	}

	return 0;
}

