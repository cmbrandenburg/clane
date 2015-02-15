// vim: set noet:

#include "check/check.h"
#include "../clane_uri_impl.hpp"

#define check_ok(in, test, exp) \
	do { \
		std::string out = "PREFIX"; \
		clane::uri::percent_encode(out, in, test); \
		check(out == std::string("PREFIX") + exp); \
	} while (false)

int main() {
	auto isalpha = [](char c) -> bool { return std::isalpha(c); };
	check_ok("", isalpha, "");
	check_ok("abcdef", isalpha, "abcdef");
	check_ok("abc123", isalpha, "abc%31%32%33");
}

