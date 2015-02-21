// vim: set noet:

#include "check/check.h"
#include "../clane_uri.hxx"

#define check_ok(in, exp) \
	do { \
		std::string s(in); \
		clane::uri::remove_dot_segments(s); \
		check(s == exp); \
	} while (false)

int main() {
	check_ok("", "");
	check_ok("alpha", "alpha");
	check_ok("/alpha", "/alpha");
	check_ok("/alpha/", "/alpha/");
	check_ok("../alpha", "alpha");
	check_ok("./alpha", "alpha");
	check_ok("/../alpha", "/alpha");
	check_ok("/./alpha", "/alpha");
	check_ok("/alpha/bravo/.", "/alpha/bravo/");
	check_ok("/alpha/bravo/..", "/alpha/");
	check_ok("/alpha/bravo", "/alpha/bravo");
	check_ok("/alpha/bravo/", "/alpha/bravo/");
	check_ok("/a/b/c/./../../g", "/a/g");
	check_ok("/content=5/../6", "/6");
}

