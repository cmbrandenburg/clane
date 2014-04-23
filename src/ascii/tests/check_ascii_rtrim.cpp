// vim: set noet:

#include "../../clane_check.hpp"
#include "../clane_ascii.hpp"

using namespace clane;

#define check_ok(in, exp) \
	do { \
		std::string s(in); \
		ascii::rtrim(s); \
		check(s == exp); \
	} while (false)

int main() {
	check_ok("", "");
	check_ok("alpha", "alpha");
	check_ok("alpha bravo", "alpha bravo");
	check_ok("alpha bravo ", "alpha bravo");
	check_ok("alpha bravo\t", "alpha bravo");
	check_ok("alpha bravo\n", "alpha bravo");
	check_ok("alpha bravo\r", "alpha bravo");
	check_ok("alpha bravo \t\n\r", "alpha bravo");
}

