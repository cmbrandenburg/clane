// vim: set noet:

#include "../uri.cpp"
#include "../../check/check.h"

int main() {
	check_true(clane::uri::is_port("1234"));
	check_true(clane::uri::is_port("1234567890")); // long values are okay
	check_true(clane::uri::is_port(""));
	check_false(clane::uri::is_port("1234alpha"));
	check_false(clane::uri::is_port("alpha1234"));
	check_false(clane::uri::is_port("alpha"));
	check_false(clane::uri::is_port("1234+"));
	return 0;
}

