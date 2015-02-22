// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// vim: set noet:

#include "check/clane_check.hxx"
#include "../clane_uri.hxx"
#include <cstring>

#define check_ok(in, tst) check(clane::uri::is_percent_encoded(in, in+std::strlen(in), tst))
#define check_nok(in, tst) check(!clane::uri::is_percent_encoded(in, in+std::strlen(in), tst))

int main() {
	check_ok("", isalpha);
	check_ok("hello", isalpha);
	check_nok("hello123", isalpha);
	check_ok("hell%6f", isalpha);
	check_ok("%68ello", isalpha);
	check_ok("%68%65%6c%6c%6f", isalpha);
	check_nok("hello%", isalpha);
	check_nok("hello%6", isalpha);
	check_nok("hello%6g", isalpha);
	check_nok("hello%g0", isalpha);
}

