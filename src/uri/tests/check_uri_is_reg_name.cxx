// vim: set noet:

#include "check/check.h"
#include "../clane_uri.hxx"
#include <cstring>

#define check_ok(in) check(clane::uri::is_reg_name(in, in+strlen(in)))
#define check_nok(in) check(!clane::uri::is_reg_name(in, in+strlen(in)))

int main() {
	check_ok("");
	check_ok("alpha.bravo+charlie");
	check_nok("alpha/bravo");
}

