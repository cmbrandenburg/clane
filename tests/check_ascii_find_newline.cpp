// vim: set noet:

#include "clane_check.hpp"
#include "../clane_ascii.hpp"
#include <cstring>

using namespace clane;

#define check_ok(bef, aft) \
	do { \
		std::string s(bef); \
		s += aft; \
		char const *got = ascii::find_newline(&s[0], &s[0]+s.size()); \
		check(got == s.data()+std::strlen(bef)); \
	} while (false)

int main() {
	check_ok("", "");
	check_ok("alpha bravo\tcharlie", "");
	check_ok("alpha", "\nbravo");
	check_ok("alpha", "\r\nbravo");
	check_ok("alpha\rbravo", "\ncharlie");
	check_ok("alpha\rbravo", "\r\ncharlie");
	check_ok("alpha", "\n");
	check_ok("alpha", "\r\n");
	check_ok("alpha", "\r");
	check_ok("alpha", "\nbravo\ncharlie");
	check_ok("alpha", "\r\nbravo\ncharlie");
	check_ok("alpha", "\nbravo\r\ncharlie");
	check_ok("alpha", "\r\nbravo\r\ncharlie");
}

