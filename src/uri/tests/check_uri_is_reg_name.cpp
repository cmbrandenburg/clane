// vim: set noet:

#include "check/check.h"

#define check_ok(in) check(uri::is_reg_name(in, in+strlen(in)))
#define check_nok(in) check(!uri::is_reg_name(in, in+strlen(in)))

int main() {
#if 0
	check_ok("");
	check_ok("alpha.bravo+charlie");
	check_nok("alpha/bravo");
#endif
	return 1;
}

