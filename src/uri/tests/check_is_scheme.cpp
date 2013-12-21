// vim: set noet:

#include "../uri.cpp"
#include "../../check/check.h"

int main() {
	check(clane::uri::is_scheme("alpha"));
	check(clane::uri::is_scheme("alpha1234"));
	check(clane::uri::is_scheme("alpha+"));
	check(clane::uri::is_scheme("alpha-"));
	check(clane::uri::is_scheme("alpha."));
	check(!clane::uri::is_scheme(""));
	check(!clane::uri::is_scheme("1234alpha"));
	check(!clane::uri::is_scheme("+alpha"));
	check(!clane::uri::is_scheme("-alpha"));
	check(!clane::uri::is_scheme(".alpha"));
	return 0;
}

