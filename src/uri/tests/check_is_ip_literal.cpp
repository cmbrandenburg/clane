// vim: set noet:

#include "../uri.cpp"
#include "../../check/check.h"

int main() {
	check(clane::uri::is_ip_literal("[::1]"));
	check(clane::uri::is_ip_literal("[v7.::1]"));
	check(!clane::uri::is_ip_literal("[invalid_address]"));
	check(!clane::uri::is_ip_literal("[127.0.0.1]"));
	return 0;
}

