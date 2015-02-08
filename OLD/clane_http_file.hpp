// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// vim: set noet:

#ifndef CLANE_HTTP__FILE_HPP
#define CLANE_HTTP__FILE_HPP

/** @file */

#include "clane_base.hpp"
#include "clane_http_server.hpp"
#include "include/clane_http_file.hpp"

namespace clane {
	namespace http {

		void serve_dir(response_ostream &rs, request &req, boost::filesystem::path const &path);
		void serve_file(response_ostream &rs, request &req, boost::filesystem::path const &path);

	}
}

#endif // #ifndef CLANE_HTTP__FILE_HPP
