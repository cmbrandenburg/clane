// vim: set noet:

#include "../http_header.h"
#include "../../check/check.h"

int main() {

	clane::http::header_name_less less;

	check(less("alpha", "bravo"));
	check(less("Alpha", "bravo"));
	check(less("alpha", "Bravo"));
	check(less("Alpha", "Bravo"));

	check(!less("bravo", "alpha"));
	check(!less("Bravo", "alpha"));
	check(!less("bravo", "Alpha"));
	check(!less("Bravo", "Alpha"));

	check(less("alpha", "charlie"));
	check(less("Alpha", "charlie"));
	check(less("alpha", "Charlie"));
	check(less("Alpha", "Charlie"));

	check(!less("charlie", "alpha"));
	check(!less("Charlie", "alpha"));
	check(!less("charlie", "Alpha"));
	check(!less("Charlie", "Alpha"));

	return 0;
}

