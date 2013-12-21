// vim: set noet:

#include "../uri.cpp"
#include "../../check/check.h"

static void check_ok(char const *exp_result, char const *s) {
	std::string got_result(s);
	bool ok = clane::uri::decode_user_info(got_result);
	check(ok);
	check(exp_result == got_result);
}

static void check_nok(char const *s) {
	std::string got_result(s);
	bool ok = clane::uri::decode_user_info(got_result);
	check(!ok);
}

int main() {
	check_ok("alpha", "alpha");
	check_ok("", "");
	check_ok("alpha bravo", "alpha%20bravo");
	check_ok("alpha:bravo", "alpha:bravo");
	check_nok("alpha%bravo");
	check_nok("alpha@");
	return 0;
}
