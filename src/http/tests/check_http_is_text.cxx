// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// vim: set noet:

#include "check/clane_check.hxx"
#include "http/clane_http_message.hxx"
#include <cstring>

#define check_ok(in) \
	do { \
		check(clane::http::is_text(in, in+std::strlen(in))); \
	} while (false)

#define check_nok(in) \
	do { \
		check(!clane::http::is_text(in, in+std::strlen(in))); \
	} while (false)

int main() {
	check_ok("alpha bravo\tcharlie<>{}[]");
	check_nok("\x01\n\x7f");
}

