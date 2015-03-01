// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// vim: set noet:

#include "ascii/clane_ascii.hxx"
#include "check/clane_check.hxx"

#define check_ok(in, key) \
	do { \
		check(clane::ascii::has_prefix(in, in+std::strlen(in), key)); \
	} while (false)

#define check_nok(in, key) \
	do { \
		check(!clane::ascii::has_prefix(in, in+std::strlen(in), key)); \
	} while (false)

int main() {
	check_ok("hello", "");
	check_ok("hello", "he");
	check_ok("hello", "hello");
	check_nok("hello", "hello there");
	check_nok("hello", "hello there");
	check_nok("", "hello");
}

