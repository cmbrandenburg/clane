// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// vim: set noet:

#include "check/clane_check.hxx"
#include "uri/clane_uri.hxx"

#define check_ok(in, test, exp) \
	do { \
		std::string out = "PREFIX"; \
		clane::uri::percent_encode(out, in, test); \
		check(out == std::string("PREFIX") + exp); \
	} while (false)

int main() {
	auto isalpha = [](char c) -> bool { return std::isalpha(c); };
	check_ok("", isalpha, "");
	check_ok("abcdef", isalpha, "abcdef");
	check_ok("abc123", isalpha, "abc%31%32%33");
}

