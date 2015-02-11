// vim: set noet:

#include "check/check.h"

#define check_ok(in) check(uri::is_path(in, in+strlen(in)))
#define check_nok(in) check(!uri::is_path(in, in+strlen(in)))

int main() {
#if 0
	check_ok("");
	check_ok("alpha.bravo+charlie");
	check_ok("alpha/bravo");
	check_ok("alpha%20bravo");
	check_nok("alpha/bravo?");
	check_nok("alpha/bravo#");
	check_nok("alpha%XXbravo");
#endif
	return 1;
}

