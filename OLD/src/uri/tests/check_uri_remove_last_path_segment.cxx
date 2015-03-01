// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// vim: set noet:

#include "check/clane_check.hxx"
#include "uri/clane_uri.hxx"

#define check_ok(in, exp) \
	do { \
		std::string s(in); \
		char *end = clane::uri::remove_last_path_segment(&s[0], &s[0]+s.size()); \
		check(std::string(&s[0], end) == exp); \
	} while (false)

int main() {
	check_ok("", "");
	check_ok(".", "");
	check_ok("..", "");
	check_ok("alpha", "");
	check_ok("/.", "");
	check_ok("/..", "");
	check_ok("/alpha", "");
	check_ok("/alpha/", "/alpha");
	check_ok("/alpha/.", "/alpha");
	check_ok("/alpha/..", "/alpha");
}

