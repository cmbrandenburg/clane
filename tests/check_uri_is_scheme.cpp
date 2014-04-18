// vim: set noet:

#include "clane_check.hpp"
#include "../clane_uri.hpp"
#include <cstring>

using namespace clane;

#define check_ok(in) check(uri::is_scheme(in, in+strlen(in)))
#define check_nok(in) check(!uri::is_scheme(in, in+strlen(in)))

int main() {

	check_ok("http");
	check_ok("mailto");
	check_ok("abcABC123+-."); // special characters

	check_nok("");
	check_nok("123abc");
	check_nok("+abc");
	check_nok("-abc");
	check_nok(".abc");
}

