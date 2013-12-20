// vim: set noet:

#include "../uri.cpp"
#include "../../check/check.h"

int main() {
	check_true(clane::uri::is_ipvfut("v7.alpha"));
	check_true(clane::uri::is_ipvfut("v777.alpha"));
	check_false(clane::uri::is_ipvfut("v777."));
	check_false(clane::uri::is_ipvfut("v777alpha"));
	check_false(clane::uri::is_ipvfut("v.alpha"));
	check_false(clane::uri::is_ipvfut("777.alpha"));
	check_false(clane::uri::is_ipvfut(""));
	return 0;
}

