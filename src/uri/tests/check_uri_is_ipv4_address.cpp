// vim: set noet:

#include "check/check.h"
#include "../clane_uri_parse.hpp"
#include <cstring>

#define check_ok(in) check(clane::uri::is_ipv4_address(in, in+strlen(in)))
#define check_nok(in) check(!clane::uri::is_ipv4_address(in, in+strlen(in)))

int main() {
	check_ok("0.0.0.0");
	check_ok("255.255.255.255");
	check_ok("127.0.0.1");
	check_ok("192.168.1.42");
	check_ok("1.2.3.4");
	check_nok("");
	check_nok("0.0.0.");
	check_nok("0.0.0");
	check_nok(".0.0.0");
	check_nok("0.0.0.0.");
	check_nok("0.0.0.0a");
	check_nok("a0.0.0.0");
	check_nok("0.a.0.0");
	check_nok("-1.2.3.4");
	check_nok("1.-2.3.4");
	check_nok("1.2.-3.4");
	check_nok("1.2.3.-4");
}

