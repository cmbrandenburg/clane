// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// vim: set noet:

#ifndef CLANE_HTTP_FILE_HPP
#define CLANE_HTTP_FILE_HPP

/** @file */

#include "clane_base_pub.hpp"
#include "clane_http_pub.hpp"
#include <boost/filesystem.hpp>

namespace clane {
	namespace http {

		class file_server {
			boost::filesystem::path root_path;
		public:
			~file_server() = default;
			file_server(boost::filesystem::path const &root_path): root_path{root_path} {}
			file_server(file_server const &) = default;
			file_server &operator=(file_server const &) = default;
#ifndef CLANE_HAVE_NO_DEFAULT_MOVE
			file_server(file_server &&) = default;
			file_server &operator=(file_server &&) = default;
#else
			file_server(file_server &&that) noexcept { swap(that); }
			file_server &operator=(file_server &&that) noexcept { swap(that); return *this; }
#endif
			void swap(file_server &that) noexcept;
			void operator()(response_ostream &rs, request &req);
		};

		inline void file_server::swap(file_server &that) noexcept {
			std::swap(root_path, that.root_path);
		}
	}
}

#endif // #ifndef CLANE_HTTP_FILE_HPP
