// vim: set noet:

#include "../uri.cpp"
#include "../../check/check.h"

int main() {

	// empty:
	check_true(clane::uri::is_ipv6_addr("::"));

	// loopback:
	check_true(clane::uri::is_ipv6_addr("0000:0000:0000:0000:0000:0000:0000:0001"));
	check_true(clane::uri::is_ipv6_addr("0:0:0:0:0:0:0:1"));
	check_true(clane::uri::is_ipv6_addr("::1"));
	check_true(clane::uri::is_ipv6_addr("::0:1"));
	check_true(clane::uri::is_ipv6_addr("::0:0:1"));
	check_true(clane::uri::is_ipv6_addr("::0:0:0:1"));
	check_true(clane::uri::is_ipv6_addr("::0:0:0:0:1"));
	check_true(clane::uri::is_ipv6_addr("::0:0:0:0:0:1"));
	check_true(clane::uri::is_ipv6_addr("::0:0:0:0:0:0:1"));
	check_true(clane::uri::is_ipv6_addr("0::0:0:0:0:0:1"));
	check_true(clane::uri::is_ipv6_addr("0:0::0:0:0:0:1"));
	check_true(clane::uri::is_ipv6_addr("0:0:0::0:0:0:1"));
	check_true(clane::uri::is_ipv6_addr("0:0:0:0::0:0:1"));
	check_true(clane::uri::is_ipv6_addr("0:0:0:0:0::0:1"));
	check_true(clane::uri::is_ipv6_addr("0:0:0:0:0:0::1"));
	check_true(clane::uri::is_ipv6_addr("0:0:0:0:0::1"));
	check_true(clane::uri::is_ipv6_addr("0:0:0:0::1"));
	check_true(clane::uri::is_ipv6_addr("0:0:0::1"));
	check_true(clane::uri::is_ipv6_addr("0:0::1"));
	check_true(clane::uri::is_ipv6_addr("0::1"));

	// empty string:
	check_false(clane::uri::is_ipv6_addr(""));

	// invalid digits:
	check_false(clane::uri::is_ipv6_addr("z000::1"));
	check_false(clane::uri::is_ipv6_addr("000z::1"));
	check_false(clane::uri::is_ipv6_addr("::z000"));
	check_false(clane::uri::is_ipv6_addr("::000z"));

	// too many digits:
	check_false(clane::uri::is_ipv6_addr("::00000"));
	check_false(clane::uri::is_ipv6_addr("00000::"));

	// too few digits:
	check_false(clane::uri::is_ipv6_addr("::0::"));
	check_false(clane::uri::is_ipv6_addr("::0::0"));
	check_false(clane::uri::is_ipv6_addr(":::0"));

	// ends with IPv4 address:
	check_true(clane::uri::is_ipv6_addr("0:0:0:0:0:0:127.0.0.1"));
	check_true(clane::uri::is_ipv6_addr("::0:0:0:0:0:127.0.0.1"));
	check_true(clane::uri::is_ipv6_addr("0::127.0.0.1"));
	check_true(clane::uri::is_ipv6_addr("::127.0.0.1"));

	// IPv4 address not at end:
	check_false(clane::uri::is_ipv6_addr("0:0:0:0:0:127.0.0.1:0"));
	check_false(clane::uri::is_ipv6_addr("::127.0.0.1:0"));

	return 0;
}

