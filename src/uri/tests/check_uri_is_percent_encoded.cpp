// vim: set noet:

#include "../../clane_check.hpp"
#include "../clane_uri.hpp"
#include <cctype>
#include <cstring>

using namespace clane;

#define check_ok(in, tst) check(uri::is_percent_encoded(in, in+std::strlen(in), tst))
#define check_nok(in, tst) check(!uri::is_percent_encoded(in, in+std::strlen(in), tst))

int main() {
	check_ok("", isalpha);
	check_ok("hello", isalpha);
	check_nok("hello123", isalpha);
	check_ok("hell%6f", isalpha);
	check_ok("%68ello", isalpha);
	check_ok("%68%65%6c%6c%6f", isalpha);
	check_nok("hello%", isalpha);
	check_nok("hello%6", isalpha);
	check_nok("hello%6g", isalpha);
	check_nok("hello%g0", isalpha);
}

