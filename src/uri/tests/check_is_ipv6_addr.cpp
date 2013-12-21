// vim: set noet:

#include "../uri.cpp"
#include "../../check/check.h"

int main() {

	// empty:
	check(clane::uri::is_ipv6_addr("::"));

	// loopback:
	check(clane::uri::is_ipv6_addr("0000:0000:0000:0000:0000:0000:0000:0001"));
	check(clane::uri::is_ipv6_addr("0:0:0:0:0:0:0:1"));
	check(clane::uri::is_ipv6_addr("::1"));
	check(clane::uri::is_ipv6_addr("::0:1"));
	check(clane::uri::is_ipv6_addr("::0:0:1"));
	check(clane::uri::is_ipv6_addr("::0:0:0:1"));
	check(clane::uri::is_ipv6_addr("::0:0:0:0:1"));
	check(clane::uri::is_ipv6_addr("::0:0:0:0:0:1"));
	check(clane::uri::is_ipv6_addr("::0:0:0:0:0:0:1"));
	check(clane::uri::is_ipv6_addr("0::0:0:0:0:0:1"));
	check(clane::uri::is_ipv6_addr("0:0::0:0:0:0:1"));
	check(clane::uri::is_ipv6_addr("0:0:0::0:0:0:1"));
	check(clane::uri::is_ipv6_addr("0:0:0:0::0:0:1"));
	check(clane::uri::is_ipv6_addr("0:0:0:0:0::0:1"));
	check(clane::uri::is_ipv6_addr("0:0:0:0:0:0::1"));
	check(clane::uri::is_ipv6_addr("0:0:0:0:0::1"));
	check(clane::uri::is_ipv6_addr("0:0:0:0::1"));
	check(clane::uri::is_ipv6_addr("0:0:0::1"));
	check(clane::uri::is_ipv6_addr("0:0::1"));
	check(clane::uri::is_ipv6_addr("0::1"));

	// empty string:
	check(!clane::uri::is_ipv6_addr(""));

	// invalid digits:
	check(!clane::uri::is_ipv6_addr("z000::1"));
	check(!clane::uri::is_ipv6_addr("000z::1"));
	check(!clane::uri::is_ipv6_addr("::z000"));
	check(!clane::uri::is_ipv6_addr("::000z"));

	// too many digits:
	check(!clane::uri::is_ipv6_addr("::00000"));
	check(!clane::uri::is_ipv6_addr("00000::"));

	// too few digits:
	check(!clane::uri::is_ipv6_addr("::0::"));
	check(!clane::uri::is_ipv6_addr("::0::0"));
	check(!clane::uri::is_ipv6_addr(":::0"));

	// ends with IPv4 address:
	check(clane::uri::is_ipv6_addr("0:0:0:0:0:0:127.0.0.1"));
	check(clane::uri::is_ipv6_addr("::0:0:0:0:0:127.0.0.1"));
	check(clane::uri::is_ipv6_addr("0::127.0.0.1"));
	check(clane::uri::is_ipv6_addr("::127.0.0.1"));

	// IPv4 address not at end:
	check(!clane::uri::is_ipv6_addr("0:0:0:0:0:127.0.0.1:0"));
	check(!clane::uri::is_ipv6_addr("::127.0.0.1:0"));

	return 0;
}

