// vim: set noet:

#include "check/check.h"
#include "../clane_uri_parse.hpp"

#define check_ok(in, exp) \
	do { \
		std::string s(in); \
		char *end = clane::uri::remove_last_path_segment(&s[0], &s[0]+s.size()); \
		check(std::string(&s[0], end) == exp); \
	} while (false)

int main() {
	check_ok("", "");
	check_ok(".", "");
	check_ok("..", "");
	check_ok("alpha", "");
	check_ok("/.", "");
	check_ok("/..", "");
	check_ok("/alpha", "");
	check_ok("/alpha/", "/alpha");
	check_ok("/alpha/.", "/alpha");
	check_ok("/alpha/..", "/alpha");
}

