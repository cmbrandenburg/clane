// vim: set noet:

#include "../net.h"
#include "../../check/check.h"
#include <sstream>
#include <stdexcept>
#include <sys/eventfd.h>
#include <sys/fcntl.h>

namespace check {

	bool is_file_descriptor_open(int fd);
	int open_file_descriptor();

	bool is_file_descriptor_open(int fd) {
		int status = fcntl(fd, F_GETFD);
		if (-1 == status && EBADF == errno)
			return false;
		if (-1 == status) {
			std::ostringstream ss;
			ss << "error getting file descriptor flags: " << clane::net::errno_to_string(errno);
			throw std::runtime_error(ss.str());
		}
		return true;
	}

	int open_file_descriptor() {
		int fd = eventfd(0, 0);
		if (-1 == fd) {
			std::ostringstream ss;
			ss << "error opening event file descriptor: " << clane::net::errno_to_string(errno);
			throw std::runtime_error(ss.str());
		}
		return fd;
	};
}

int main() {
	using namespace check;
	using clane::net::file_descriptor;

	// default constructor: */
	{
		file_descriptor fd;
	}

	// set-ownership constructor: */
	{
		file_descriptor fd(open_file_descriptor());
		check_true(is_file_descriptor_open(fd));
	}

	// move constructor:
	{
		file_descriptor fd1(open_file_descriptor());
		file_descriptor fd2 = std::move(fd1);
		check_false(is_file_descriptor_open(fd1));
		check_true(is_file_descriptor_open(fd2));
	}

	// move assignment:
	{
		file_descriptor fd1(open_file_descriptor());
		file_descriptor fd2;
		fd2 = std::move(fd1);
		check_false(is_file_descriptor_open(fd1));
		check_true(is_file_descriptor_open(fd2));
	}

	// set-ownership assignment: */
	{
		file_descriptor fd;
		int sys_fd = open_file_descriptor();
		fd = sys_fd;
		check_true(is_file_descriptor_open(fd));
	}

	// explicit close:
	{
		int sys_fd = open_file_descriptor();
		file_descriptor fd(sys_fd);
		fd.close();
		check_false(is_file_descriptor_open(sys_fd));
	}

	// detach:
	{
		file_descriptor fd(open_file_descriptor());
		int sys_fd = fd.detach();
		check_eq(-1, int{fd});
		fd = sys_fd;
	}

	return 0;
}

