// vim: set noet:

#include "../uri.cpp"
#include "../../check/check.h"

int main() {
	check(clane::uri::is_port("1234"));
	check(clane::uri::is_port("1234567890")); // long values are okay
	check(clane::uri::is_port(""));
	check(!clane::uri::is_port("1234alpha"));
	check(!clane::uri::is_port("alpha1234"));
	check(!clane::uri::is_port("alpha"));
	check(!clane::uri::is_port("1234+"));
	return 0;
}

