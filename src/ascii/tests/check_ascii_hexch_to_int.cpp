// vim: set noet:

#include "check/check.h"
#include "../clane_ascii_impl.hpp"
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

