// vim: set noet:

#include "../../check/check.h"
#include "../uri.cpp"

int main() {
	check(clane::uri::is_ipvfut("v7.alpha"));
	check(clane::uri::is_ipvfut("v777.alpha"));
	check(!clane::uri::is_ipvfut("v777."));
	check(!clane::uri::is_ipvfut("v777alpha"));
	check(!clane::uri::is_ipvfut("v.alpha"));
	check(!clane::uri::is_ipvfut("777.alpha"));
	check(!clane::uri::is_ipvfut(""));
	return 0;
}

