// vim: set noet:

#include "clane_check.hpp"
#include "../clane_uri.hpp"
#include <cstring>

using namespace clane;

#define check_ok(in) check(uri::is_ipv6_address(in, in+strlen(in)))
#define check_nok(in) check(!uri::is_ipv6_address(in, in+strlen(in)))

int main() {

	// empty:
	check_ok("::");

	// loopback:
	check_ok("0000:0000:0000:0000:0000:0000:0000:0001");
	check_ok("0:0:0:0:0:0:0:1");
	check_ok("::1");
	check_ok("::0:1");
	check_ok("::0:0:1");
	check_ok("::0:0:0:1");
	check_ok("::0:0:0:0:1");
	check_ok("::0:0:0:0:0:1");
	check_ok("::0:0:0:0:0:0:1");
	check_ok("0::0:0:0:0:0:1");
	check_ok("0:0::0:0:0:0:1");
	check_ok("0:0:0::0:0:0:1");
	check_ok("0:0:0:0::0:0:1");
	check_ok("0:0:0:0:0::0:1");
	check_ok("0:0:0:0:0:0::1");
	check_ok("0:0:0:0:0::1");
	check_ok("0:0:0:0::1");
	check_ok("0:0:0::1");
	check_ok("0:0::1");
	check_ok("0::1");

	// empty string:
	check_nok("");

	// invalid digits:
	check_nok("z000::1");
	check_nok("000z::1");
	check_nok("::z000");
	check_nok("::000z");

	// too many digits:
	check_nok("::00000");
	check_nok("00000::");

	// too few digits:
	check_nok("::0::");
	check_nok("::0::0");
	check_nok(":::0");

	// ends with IPv4 address:
	check_ok("0:0:0:0:0:0:127.0.0.1");
	check_ok("::0:0:0:0:0:127.0.0.1");
	check_ok("0::127.0.0.1");
	check_ok("::127.0.0.1");

	// IPv4 address not at end:
	check_nok("0:0:0:0:0:127.0.0.1:0");
	check_nok("::127.0.0.1:0");

}

