// vim: set noet:

#ifndef CLANE__NET_POSIX_HPP
#define CLANE__NET_POSIX_HPP

#include "net_common.hpp"
#include <string>

namespace clane {
	namespace posix {

		std::string errno_to_string(int n);

		void close_fd(int fd);
		void set_nonblocking(int fd);

		class file_descriptor {
		protected:
			int fd;
		public:
			~file_descriptor() { close(); }
			file_descriptor() noexcept: fd(-1) {}
			file_descriptor(file_descriptor const &) = delete;
			file_descriptor(file_descriptor &&that) noexcept: fd(-1) { swap(that); }
			file_descriptor(int that_fd) noexcept: fd(that_fd) {}
			file_descriptor &operator=(file_descriptor const &) = delete;
			file_descriptor &operator=(file_descriptor &&that) noexcept;
			file_descriptor &operator=(int that_fd) noexcept;
			void swap(file_descriptor &that) noexcept { std::swap(fd, that.fd); }
			operator int() const noexcept { return fd; }
			void close();
			int release() noexcept;
		};

		inline file_descriptor &file_descriptor::operator=(file_descriptor &&that) noexcept {
			swap(that);
			return *this;
		}

		inline file_descriptor &file_descriptor::operator=(int that_fd) noexcept {
			close();
			fd = that_fd;
			return *this;
		}

		inline int file_descriptor::release() noexcept {
			int n = fd;
			fd = -1;
			return n;
		}

		inline void swap(file_descriptor &a, file_descriptor &b) noexcept {
			a.swap(b);
		}
	}
}

#endif // #ifndef CLANE__NET_POSIX_HPP
