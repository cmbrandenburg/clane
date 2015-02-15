// vim: set noet:

#include "check/check.h"
#include "../clane_uri_impl.hpp"
#include <cstring>

#define check_ok(in) check(clane::uri::is_ipvfut_address(in, in+strlen(in)))
#define check_nok(in) check(!clane::uri::is_ipvfut_address(in, in+strlen(in)))

int main() {
	check_ok("v7.alpha");
	check_ok("v777.alpha");
	check_nok("v777.");
	check_nok("v777alpha");
	check_nok("v.alpha");
	check_nok("777.alpha");
	check_nok("");
}

