// vim: set noet:

#include "clane_check.hpp"
#include "../clane_uri.cpp"
#include <cctype>

using namespace clane;

#define check_ok(in, test, exp) \
	do { \
		std::string out = "PREFIX"; \
		uri::percent_encode(out, in, test); \
		check(out == std::string("PREFIX") + exp); \
	} while (false)

int main() {
	check_ok("", isalpha, "");
	check_ok("abcdef", isalpha, "abcdef");
	check_ok("abc123", isalpha, "abc%31%32%33");
}

