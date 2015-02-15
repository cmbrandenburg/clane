// vim: set noet:

#include "check/check.h"
#include "clane/clane_ascii.hpp"

#define check_eq(a, b) \
	do { \
		check(clane::ascii::icase_compare(a, b) == 0); \
	} while (false)

#define check_lt(a, b) \
	do { \
		check(clane::ascii::icase_compare(a, b) < 0); \
	} while (false)

#define check_gt(a, b) \
	do { \
		check(clane::ascii::icase_compare(a, b) > 0); \
	} while (false)

int main() {
	check_eq("", "");
	check_lt("", "alpha");
	check_gt("alpha", "");
	check_eq("alpha", "alpha");
	check_eq("ALPHA", "alpha");
	check_eq("alpha", "ALPHA");
	check_eq("AlphA", "aLPHa");
	check_gt("AlphAs", "aLPHa");
	check_lt("AlphA", "aLPHas");
}

