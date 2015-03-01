// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// vim: set noet:

#include "check/clane_check.hxx"
#include "uri/clane_uri.hxx"
#include <cstring>

#define check_ok(in) check(clane::uri::is_scheme(in, in+strlen(in)))
#define check_nok(in) check(!clane::uri::is_scheme(in, in+strlen(in)))

int main() {
	check_ok("http");
	check_ok("mailto");
	check_ok("abcABC123+-."); // special characters
	check_nok("");
	check_nok("123abc");
	check_nok("+abc");
	check_nok("-abc");
	check_nok(".abc");
}
