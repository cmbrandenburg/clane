// vim: set noet:

#include "check/check.h"

#define check_ok(in, exp) \
	do { \
		std::string s(in); \
		uri::remove_empty_segments(s); \
		check(s == exp); \
	} while (false)

int main() {
#if 0
	check_ok("", "");
	check_ok("/alpha/bravo/", "/alpha/bravo/");
	check_ok("//alpha/bravo/", "/alpha/bravo/");
	check_ok("/alpha//bravo/", "/alpha/bravo/");
	check_ok("/alpha/bravo//", "/alpha/bravo/");
	check_ok("//alpha//bravo//", "/alpha/bravo/");
#endif
	return 1;
}

