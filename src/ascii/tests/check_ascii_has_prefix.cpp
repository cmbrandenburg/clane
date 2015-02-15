// vim: set noet:

#include "check/check.h"
#include "../clane_ascii.hpp"

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

