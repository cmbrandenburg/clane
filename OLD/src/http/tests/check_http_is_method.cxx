// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// vim: set noet:

#include "check/clane_check.hxx"
#include "http/clane_http_message.hxx"
#include <cstring>

#define check_ok(in) \
	do { \
		check(clane::http::is_method(in, in+std::strlen(in))); \
	} while (false)

#define check_nok(in) \
	do { \
		check(!clane::http::is_method(in, in+std::strlen(in))); \
	} while (false)

int main() {
	check_ok("GET");
	check_ok("PUT");
	check_ok("ok_method");
	check_nok("bad\nmethod");
	check_nok("bad:method");
}

