// vim: set noet:

#include "../http_header.h"
#include "../../check/check.h"

int main() {

	clane::http::header_name_less less;

	check_true(less("alpha", "bravo"));
	check_true(less("Alpha", "bravo"));
	check_true(less("alpha", "Bravo"));
	check_true(less("Alpha", "Bravo"));

	check_false(less("bravo", "alpha"));
	check_false(less("Bravo", "alpha"));
	check_false(less("bravo", "Alpha"));
	check_false(less("Bravo", "Alpha"));

	check_true(less("alpha", "charlie"));
	check_true(less("Alpha", "charlie"));
	check_true(less("alpha", "Charlie"));
	check_true(less("Alpha", "Charlie"));

	check_false(less("charlie", "alpha"));
	check_false(less("Charlie", "alpha"));
	check_false(less("charlie", "Alpha"));
	check_false(less("Charlie", "Alpha"));

	return 0;
}

