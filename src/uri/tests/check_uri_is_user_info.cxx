// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// vim: set noet:

#include "check/clane_check.hxx"
#include "../clane_uri.hxx"
#include <cstring>

#define check_ok(in) check(clane::uri::is_user_info(in, in+strlen(in)))
#define check_nok(in) check(!clane::uri::is_user_info(in, in+strlen(in)))

int main() {
	check_ok("john_doe123");
	check_ok(":'!"); // colon and sub-delimiters
	check_ok("john%20doe");
	check_nok("john/doe"); // general delimiters
	check_nok("john%ggdoe");
}

