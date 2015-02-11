// vim: set noet:

#include "check/check.h"

#define check_ok(in) check(uri::is_ls32(in, in+strlen(in)))
#define check_nok(in) check(!uri::is_ls32(in, in+strlen(in)))

int main() {

#if 0
	check_ok("0:0");
	check_ok("F:F");
	check_ok("f:f");
	check_ok("0123:89ab");

	check_ok("127.0.0.1");

	check_nok("1234:");
	check_nok(":1234");
	check_nok("12345:67");
	check_nok("1234:67890");
	check_nok("1234:5678:");
	check_nok("1234:127.0.0.1");
#endif
	return 1;
}

