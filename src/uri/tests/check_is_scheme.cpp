// vim: set noet:

#include "../uri.cpp"
#include "../../check/check.h"

int main() {
	check_true(clane::uri::is_scheme("alpha"));
	check_true(clane::uri::is_scheme("alpha1234"));
	check_true(clane::uri::is_scheme("alpha+"));
	check_true(clane::uri::is_scheme("alpha-"));
	check_true(clane::uri::is_scheme("alpha."));
	check_false(clane::uri::is_scheme(""));
	check_false(clane::uri::is_scheme("1234alpha"));
	check_false(clane::uri::is_scheme("+alpha"));
	check_false(clane::uri::is_scheme("-alpha"));
	check_false(clane::uri::is_scheme(".alpha"));
	return 0;
}

