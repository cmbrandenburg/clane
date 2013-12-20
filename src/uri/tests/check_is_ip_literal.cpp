// vim: set noet:

#include "../uri.cpp"
#include "../../check/check.h"

int main() {
	check_true(clane::uri::is_ip_literal("[::1]"));
	check_true(clane::uri::is_ip_literal("[v7.::1]"));
	check_false(clane::uri::is_ip_literal("[invalid_address]"));
	check_false(clane::uri::is_ip_literal("[127.0.0.1]"));
	return 0;
}

