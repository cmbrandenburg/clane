// vim: set noet:

#include "check/check.h"

#define check_ok(in) check(uri::is_ipvfut_address(in, in+strlen(in)))
#define check_nok(in) check(!uri::is_ipvfut_address(in, in+strlen(in)))

int main() {
#if 0
	check_ok("v7.alpha");
	check_ok("v777.alpha");
	check_nok("v777.");
	check_nok("v777alpha");
	check_nok("v.alpha");
	check_nok("777.alpha");
	check_nok("");
#endif
	return 1;
}

