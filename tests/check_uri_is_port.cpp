// vim: set noet:

#include "clane_check.hpp"
#include "../clane_uri.hpp"
#include <cstring>

using namespace clane;

#define check_ok(in) check(uri::is_port(in, in+strlen(in)))
#define check_nok(in) check(!uri::is_port(in, in+strlen(in)))

int main() {
	check_ok("");
	check_ok("80");
	check_ok("1234");
	check_nok("1234/");
	check_nok("-1234");
	check_nok("1234a");
}

