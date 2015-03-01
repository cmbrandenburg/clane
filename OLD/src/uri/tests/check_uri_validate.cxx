// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// vim: set noet:

#include "check/clane_check.hxx"
#include "uri/clane_uri.hxx"

void okay(clane::uri::uri const &u) {
	std::error_code e;
	u.validate(e);
	check(!e);
}

void not_okay(clane::uri::uri const &u) {
	std::error_code e;
	u.validate(e);
	check(e);
}

int main() {
	clane::uri::uri u;

	u.clear();
	u.host = "alpha";
	u.path = "/bravo";
	check_call(okay, u);

	// authority with relative path:
	u.clear();
	u.host = "alpha";
	u.path = "bravo";
	check_call(not_okay, u);

	// no authority
	u.clear();
	u.path = "//alpha";
	check_call(not_okay, u);

	// no scheme, no authority, first path segment with colon:
	u.clear();
	u.path = "alpha:bravo";
	check_call(not_okay, u);
}

