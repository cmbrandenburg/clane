// vim: set noet:

#include "../../clane_check.hpp"
#include "../clane_uri.hpp"

using namespace clane;

#define check_ok(in, exp) \
	do { \
		std::string s(in); \
		uri::remove_empty_segments(s); \
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

