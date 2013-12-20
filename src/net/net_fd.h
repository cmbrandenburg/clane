// vim: set noet:

#ifndef CLANE_NET_FD_H
#define CLANE_NET_FD_H

/** @file
 *
 * @brief File descriptors */

#include "net_common.h"
#include <utility>

namespace clane {
	namespace net {

		void close_fd(int fd);

		void set_nonblocking(int fd);
		void set_blocking(int fd);

		/** @brief Wraps a Linux file descriptor with RAII semantics */
		class file_descriptor {

		protected:

			/** @brief Underlying file descriptor */
			int fd;

		public:

			/** @brief Closes the underlying file descriptor */
			~file_descriptor();

			/** @brief Constructs without ownership of any underlying file descriptor */
			file_descriptor();

			file_descriptor(file_descriptor const &) = delete;

			/** @brief Constructs by transferring ownership of `that`'s underlying file
			 * descriptor to `this` */
			file_descriptor(file_descriptor &&that);

			/** @brief Constructs by taking ownership of the `that_fd` file descriptor
			 * */
			file_descriptor(int that_fd);

			file_descriptor &operator=(file_descriptor const &) = delete;

			/** @brief Assigns by transferring ownership of `that`'s underlying file
			 * descriptor to `this` */
			file_descriptor &operator=(file_descriptor &&that);

			/** @brief Assigns by taking ownership of the `that_fd` file descriptor */
			file_descriptor &operator=(int that_fd);

			/** @brief Returns whether the underlying file descriptor is set */
			operator bool() const;

			/** @brief Returns the underlying file descriptor */
			operator int() const;

			/** @brief Closes the underlying file descriptor */
			void close();

			/** @brief Drops ownership of and returns the underlying file descriptor */
			int release();

			friend void swap(file_descriptor &a, file_descriptor &b);
		};

		/** @brief Swaps two file descriptors */
		void swap(file_descriptor &a, file_descriptor &b);

		inline file_descriptor::~file_descriptor() {
			close();
		}

		inline file_descriptor::file_descriptor() : fd(-1) {
		}

		inline file_descriptor::file_descriptor(file_descriptor &&that) : fd(-1) {
			swap(*this, that);
		}

		inline file_descriptor::file_descriptor(int that_fd) : fd(that_fd) {
		}

		inline file_descriptor &file_descriptor::operator=(file_descriptor &&that) {
			swap(*this, that);
			return *this;
		}

		inline file_descriptor &file_descriptor::operator=(int that_fd) {
			close();
			fd = that_fd;
			return *this;
		}

		inline file_descriptor::operator bool() const {
			return -1 != fd;
		}

		inline file_descriptor::operator int() const {
			return fd;
		}

		inline int file_descriptor::release() {
			int released_fd = fd;
			fd = -1;
			return released_fd;
		}

		inline void swap(file_descriptor &a, file_descriptor &b) {
			std::swap(a.fd, b.fd);
		}
	}
}

#endif // #ifndef CLANE_NET_FD_H
