// vim: set noet:

#include "check/check.h"

#define check_ok(in, test, exp) \
	do { \
		std::string out = "PREFIX"; \
		uri::percent_encode(out, in, test); \
		check(out == std::string("PREFIX") + exp); \
	} while (false)

int main() {
#if 0
	check_ok("", isalpha, "");
	check_ok("abcdef", isalpha, "abcdef");
	check_ok("abc123", isalpha, "abc%31%32%33");
#endif
	return 1;
}

