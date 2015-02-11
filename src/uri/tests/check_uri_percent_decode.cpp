// vim: set noet:

#include "check/check.h"

#define check_ok(in, exp) \
	do { \
		std::string out = uri::percent_decode(&in[0], &in[0]+std::strlen(in)); \
		check(out == exp); \
	} while (false)

int main() {

#if 0
	// empty string
	check_ok("", "");

	// no encoded characters:
	check_ok("abcdef", "abcdef");

	// all encoded characters:
	check_ok("%68%65%6C%6C%6F%20%77%6F%72%6C%64", "hello world");
#endif
	return 1;
}

