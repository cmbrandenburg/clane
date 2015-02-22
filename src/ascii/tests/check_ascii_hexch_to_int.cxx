// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// vim: set noet:

#include "check/clane_check.hxx"
#include "../clane_ascii.hxx"
#include <cstring>

int main() {
	using clane::ascii::hexch_to_int;
	check(0 == hexch_to_int('0'));
	check(1 == hexch_to_int('1'));
	check(9 == hexch_to_int('9'));
	check(10 == hexch_to_int('a'));
	check(15 == hexch_to_int('f'));
	check(10 == hexch_to_int('A'));
	check(15 == hexch_to_int('F'));
}

