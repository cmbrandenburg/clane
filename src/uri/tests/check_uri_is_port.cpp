// vim: set noet:

#include "check/check.h"

#define check_ok(in) check(uri::is_port(in, in+strlen(in)))
#define check_nok(in) check(!uri::is_port(in, in+strlen(in)))

int main() {
#if 0
	check_ok("");
	check_ok("80");
	check_ok("1234");
	check_nok("1234/");
	check_nok("-1234");
	check_nok("1234a");
#endif
	return 1;
}

