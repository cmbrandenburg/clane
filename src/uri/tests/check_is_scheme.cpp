// vim: set noet:

#include "../../check/check.h"
#include "../uri.cpp"

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

