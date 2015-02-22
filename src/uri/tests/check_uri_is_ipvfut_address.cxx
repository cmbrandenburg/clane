// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// vim: set noet:

#include "check/clane_check.hxx"
#include "../clane_uri.hxx"
#include <cstring>

#define check_ok(in) check(clane::uri::is_ipvfut_address(in, in+strlen(in)))
#define check_nok(in) check(!clane::uri::is_ipvfut_address(in, in+strlen(in)))

int main() {
	check_ok("v7.alpha");
	check_ok("v777.alpha");
	check_nok("v777.");
	check_nok("v777alpha");
	check_nok("v.alpha");
	check_nok("777.alpha");
	check_nok("");
}

