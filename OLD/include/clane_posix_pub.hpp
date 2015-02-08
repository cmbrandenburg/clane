// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// vim: set noet:

#ifndef CLANE_POSIX_PUB_HPP
#define CLANE_POSIX_PUB_HPP

/** @file
 *
 * @brief POSIX utilities */

#include "clane_base_pub.hpp"
#include <utility>

namespace clane {

	namespace posix {
	
		/** @brief Closes a POSIX-style file descriptor */
		void close_fd(int fd);

		/** @brief Provides RAII semantics for a POSIX-style file descriptor */
		class unique_fd {
			int fd;

		public:

			/** @brief Closes this unique_fd, if open */
			~unique_fd() { close(); }

			/** @brief Constructs this unique_fd as closed */
			unique_fd() noexcept: fd(-1) {}

			/** @brief Constructs this unique_fd as a given file descriptor */
			unique_fd(int that_fd) noexcept: fd(that_fd) {}

			/** @brief Deleted */
			unique_fd(unique_fd const &) = delete;

			/** @brief Constructs this unique_fd by moving another */
			unique_fd(unique_fd &&that) noexcept: fd(-1) { swap(that); }

			/** @brief Deleted */
			unique_fd &operator=(unique_fd const &) = delete;

			/** @brief Assigns this unique_fd by moving another */
			unique_fd &operator=(unique_fd &&that) noexcept { swap(that); return *this; }

			/** @brief Assigns this unique_fd a given file descriptor */
			unique_fd &operator=(int that_fd) noexcept { fd = that_fd; return *this; }

			/** @brief Swaps this unique_fd with another */
			void swap(unique_fd &that) noexcept { std::swap(fd, that.fd); }

			/** @brief Returns the underlying file descriptor of this unique_fd */
			operator int() const noexcept { return fd; }

			/** @brief Closes this unique_fd, if open */
			void close();

			/** @brief Relinquishes ownership of the underlying file descriptor and
			 * returns it */
			int release() noexcept;
		};

		/** @brief Swaps two unique_fd instances */
		inline void swap(unique_fd &a, unique_fd &b) noexcept {
			a.swap(b);
		}

		inline void unique_fd::close() {

			if (-1 == fd)
				return;

			// This relinquishes ownership of the file descriptor, regardless whether
			// the close operation succeeds, so as never to try to close the same file
			// descriptor twice.

			int n = release();
			close_fd(n);
		}

		inline int unique_fd::release() noexcept {
			int n = fd;
			fd = -1;
			return n;
		}

	}

}

#endif // #ifndef CLANE_POSIX_PUB_HPP
