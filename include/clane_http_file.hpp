// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// vim: set noet:

#ifndef CLANE_HTTP_FILE_HPP
#define CLANE_HTTP_FILE_HPP

/** @file
 *
 * @brief HTTP file serving */

#include "clane_base.hpp"
#include "clane_http_message.hpp"
#include <boost/filesystem.hpp>

namespace clane {
	namespace http {

		/** @brief Serves a file or directory in the local file system
		 *
		 * @relates file_server
		 *
		 * @remark The serve_file() function writes to the @p rs argument a complete
		 * HTTP response whose content is that of the @p path argument. If @p path
		 * is a directory then the serve_file() function serves a human-readable
		 * list of all subdirectories and files in @p path. If @p path is a regular
		 * file or symbolic link then the serve_file() function serves the content
		 * of that file or link target and uses the mime::default_map global
		 * variable to determine the `content-type` header. If @p path is some other
		 * file type (e.g., a device file) then the serve_file() function responds
		 * with a 404 error (“Not found”).
		 *
		 * @remark The serve_file() function ignores the HTTP request URI path and
		 * always serve the directory or file specified by the @p path argument.
		 *
		 * @remark Unlike the file_server class, the serve_file() function is
		 * stateless.
		 *
		 * @sa file_server
		 */
		void serve_file(response_ostream &rs, request &req, boost::filesystem::path const &path);

		/** @brief @ref http_request_handling_page "HTTP request handler" that
		 * serves directories and files from the local file system
		 *
		 * @remark The file_server class responds to an HTTP request by serving a
		 * directory or file from the local file system. It uses a **root path** and
		 * the request URI path together to determine the directory or file to
		 * serve. Applications may use the file_server class to enumerate
		 * directories, recurse subdirectories, and serve single files, thereby
		 * making the file_server class useful for file servers as well as
		 * applications that serve static content, such as `.html`, `.css`, `.js`,
		 * and image files.
		 *
		 * @remark When responding to a request, the file_server class joins its
		 * root path, which is set during construction, with the HTTP request URI
		 * path to determine the name of the directory or file to serve. For
		 * example, a file_server instance with a root path of `/alpha/bravo` and a
		 * request URI path of `/charlie/delta` will attempt to serve the file
		 * `/alpha/bravo/charlie/delta`. The root path may be absolute or relative.
		 * If the root path is a file---not a directory---then the request URI path
		 * must be empty, else file_server will join the root path and URI path,
		 * thereby causing a 404 error (“Not found”). Sometimes it's more convenient
		 * for applications to use the serve_file() function to serve specific,
		 * individual files.
		 *
		 * @sa basic_prefix_stripper
		 * @sa serve_file */
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
