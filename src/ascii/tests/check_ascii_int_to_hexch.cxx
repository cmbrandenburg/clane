// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// vim: set noet:

#include "ascii/clane_ascii.hxx"
#include "check/clane_check.hxx"
#include <cstring>

int main() {
	using clane::ascii::int_to_hexch;
	check('0' == int_to_hexch(0));
	check('1' == int_to_hexch(1));
	check('9' == int_to_hexch(9));
	check('A' == int_to_hexch(10));
	check('F' == int_to_hexch(15));
}

