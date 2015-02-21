// vim: set noet:

#include "check/check.h"
#include "../clane_ascii.hxx"
#include <cstring>

int main() {
	using clane::ascii::int_to_hexch;
	check('0' == int_to_hexch(0));
	check('1' == int_to_hexch(1));
	check('9' == int_to_hexch(9));
	check('A' == int_to_hexch(10));
	check('F' == int_to_hexch(15));
}

