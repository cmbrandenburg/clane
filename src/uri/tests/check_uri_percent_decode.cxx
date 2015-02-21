// vim: set noet:

#include "check/check.h"
#include "../clane_uri.hxx"
#include <cstring>

#define check_ok(in, exp) \
	do { \
		std::string out = clane::uri::percent_decode(&in[0], &in[0]+std::strlen(in)); \
		check(out == exp); \
	} while (false)

int main() {

	// empty string
	check_ok("", "");

	// no encoded characters:
	check_ok("abcdef", "abcdef");

	// all encoded characters:
	check_ok("%68%65%6C%6C%6F%20%77%6F%72%6C%64", "hello world");

	// some characters encoded, some not:
	check_ok("a%62c%64e%66", "abcdef");
}

