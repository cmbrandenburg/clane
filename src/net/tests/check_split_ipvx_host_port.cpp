// vim: set noet:

#include "../net.cpp"
#include "../../check/check.h"

#define check(addr_in, exp_valid, exp_protocol, exp_host, exp_port) \
	do { \
		char buf[1024] = addr_in; \
		char *host{}, *port{}; \
		if (exp_valid) { \
			clane::net::protocol_family const *pf = clane::net::split_ipvx_host_port(buf, &host, &port); \
			check_eq(exp_protocol, pf->domain()); \
			check_cstr_eq(exp_host, host); \
			check_cstr_eq(exp_port, port); \
		} else { \
			check_throw(std::invalid_argument, clane::net::split_ipvx_host_port(buf, &host, &port)); \
			check_cstr_eq(addr_in, buf); \
			check_null(host); \
			check_null(port); \
		} \
	} while (false)

int main() {
	check("", false, 0, "", "");
	check("[", false, 0, "", "");
	check("]", false, 0, "", "");
	check("[]", false, 0, "", "");
	check("][", false, 0, "", "");

	// IPv6:
	check("[]:", true, AF_INET6, "", "");
	check("[alpha]:", true, AF_INET6, "alpha", "");
	check("[]:alpha", true, AF_INET6, "", "alpha");
	check("[alpha]:bravo", true, AF_INET6, "alpha", "bravo");
	check("[alpha]:", true, AF_INET6, "alpha", "");

	// IPv4:
	check(":", true, AF_INET6, "", "");
	check("alpha:", true, AF_INET6, "alpha", "");
	check(":alpha", true, AF_INET6, "", "alpha");
	check("alpha:bravo", true, AF_INET6, "alpha", "bravo");

	// whitespace errors:
	check(" []:", false, 0, "", "");
	check("[] :", false, 0, "", "");
	check("[]: ", false, 0, "", "");
	check(" [alpha]:bravo", false, 0, "", "");
	check("[alpha] :bravo", false, 0, "", "");
	check("[alpha]:bravo ", false, 0, "", "");
	check("[alpha]: bravo ", false, 0, "", "");

	// protocol family distinction:
	check("alpha:bravo", true, AF_INET6, "alpha", "bravo");
	check("[::]:bravo", true, AF_INET6, "::", "bravo");
	check("127.0.0.1:bravo", true, AF_INET, "127.0.0.1", "bravo");

	return 0;
}

