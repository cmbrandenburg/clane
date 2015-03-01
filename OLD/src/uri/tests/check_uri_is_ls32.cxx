// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// vim: set noet:

#include "check/clane_check.hxx"
#include "uri/clane_uri.hxx"
#include <cstring>

#define check_ok(in) check(clane::uri::is_ls32(in, in+strlen(in)))
#define check_nok(in) check(!clane::uri::is_ls32(in, in+strlen(in)))

int main() {
	check_ok("0:0");
	check_ok("F:F");
	check_ok("f:f");
	check_ok("0123:89ab");
	check_ok("127.0.0.1");
	check_nok("1234:");
	check_nok(":1234");
	check_nok("12345:67");
	check_nok("1234:67890");
	check_nok("1234:5678:");
	check_nok("1234:127.0.0.1");
}

