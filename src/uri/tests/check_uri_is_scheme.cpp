// vim: set noet:

#include "check/check.h"
#include "../clane_uri_impl.hpp"
#include <cstring>

#define check_ok(in) check(clane::uri::is_scheme(in, in+strlen(in)))
#define check_nok(in) check(!clane::uri::is_scheme(in, in+strlen(in)))

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

