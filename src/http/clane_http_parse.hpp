// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// vim: set noet:

#ifndef CLANE__HTTP_PARSE_HPP
#define CLANE__HTTP_PARSE_HPP

/** @file */

#include "clane_http_message.hpp"
#include "../clane_base.hpp"

namespace clane {
	namespace http {

		bool is_header_name_valid(std::string const &s);
		bool is_header_value_valid(std::string const &s);
		bool is_method_valid(std::string const &s);

		bool parse_version(int *major_ver, int *minor_ver, std::string &s);
		bool parse_status_code(status_code *stat, std::string &s);

		bool query_headers_chunked(header_map const &hdrs);
		bool query_headers_content_length(header_map const &hdrs, size_t &len_o);

	}
}

#endif // #ifndef CLANE__HTTP_PARSE_HPP
