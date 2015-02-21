// vim: set noet:

#include "check/check.h"
#include "../clane_uri.hxx"
#include <cstring>

#define check_ok(in) check(clane::uri::is_port(in, in+strlen(in)))
#define check_nok(in) check(!clane::uri::is_port(in, in+strlen(in)))

int main() {
	check_ok("");
	check_ok("80");
	check_ok("1234");
	check_nok("1234/");
	check_nok("-1234");
	check_nok("1234a");
}

