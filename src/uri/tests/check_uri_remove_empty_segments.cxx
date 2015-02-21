// vim: set noet:

#include "check/check.h"
#include "../clane_uri.hxx"

#define check_ok(in, exp) \
	do { \
		std::string s(in); \
		clane::uri::remove_empty_segments(s); \
		check(s == exp); \
	} while (false)

int main() {
	check_ok("", "");
	check_ok("/alpha/bravo/", "/alpha/bravo/");
	check_ok("//alpha/bravo/", "/alpha/bravo/");
	check_ok("/alpha//bravo/", "/alpha/bravo/");
	check_ok("/alpha/bravo//", "/alpha/bravo/");
	check_ok("//alpha//bravo//", "/alpha/bravo/");
}

