// vim: set noet:

#include "../uri.cpp"
#include "../../check/check.h"

int main() {
	check_true(clane::uri::is_ipv4_addr("127.0.0.1"));
	check_false(clane::uri::is_ipv4_addr("256.0.0.1"));
	check_false(clane::uri::is_ipv4_addr("-1.0.0.1"));
	check_false(clane::uri::is_ipv4_addr("a.0.0.1"));
	check_false(clane::uri::is_ipv4_addr("127.0.0.256"));
	check_false(clane::uri::is_ipv4_addr("127.0.0.-1"));
	check_false(clane::uri::is_ipv4_addr("127.0.0.a"));
	check_false(clane::uri::is_ipv4_addr("127.0.0."));
	check_false(clane::uri::is_ipv4_addr("127.0.0"));
	check_false(clane::uri::is_ipv4_addr(""));
	return 0;
}

