// vim: set noet:

#include "../uri.cpp"
#include "../../check/check.h"

#define check_ok(exp_result, s) \
	do { \
		check_(__FILE__, __LINE__, true, exp_result, s); \
	} while (false)
#define check_nok(s) \
	do { \
		check_(__FILE__, __LINE__, false, "", s); \
	} while (false)

static void check_(char const *sname, int sline, bool exp_ok, char const *exp_result, char const *s);

void check_(char const *sname, int sline, bool exp_ok, char const *exp_result, char const *s) {
	std::string got_result(s);
	bool ok = clane::uri::decode_query(got_result);
	if (exp_ok) {
		clane::check::check_true_(sname, sline, ok);
		clane::check::check_eq_(sname, sline, exp_result, got_result);
	} else {
		clane::check::check_false_(sname, sline, ok);
	}
}

int main() {
	check_ok("", "");
	check_ok("alpha", "alpha");
	check_ok("alpha bravo", "alpha%20bravo");
	check_ok("alpha?bravo=charlie", "alpha?bravo=charlie");
	check_ok("alpha?bravo=/charlie", "alpha?bravo=/charlie");
	check_nok("alpha%bravo");
	return 0;
}

