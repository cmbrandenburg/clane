// vim: set noet:

#include "../../check/check.h"
#include "../uri.cpp"

static void check_ok(char const *exp_result, char const *s) {
	std::string got_result(s);
	clane::uri::normalize_path(got_result);
	check(exp_result == got_result);
}

int main() {

	check_ok("", "");
	check_ok("/", "/");
	check_ok("alpha", "alpha");
	check_ok("alpha/bravo/charlie", "alpha/bravo/charlie");

	// trailing slash is preserved:
	check_ok("alpha/bravo/charlie/", "alpha/bravo/charlie/");

	// absolute path:
	check_ok("/alpha/bravo/charlie", "/alpha/bravo/charlie");
	check_ok("/alpha/bravo/charlie/", "/alpha/bravo/charlie/");

	// single dot at beginning:
	check_ok("", ".");
	check_ok("", "./");
	check_ok("alpha", "./alpha");
	check_ok("/alpha", "/./alpha");

	// double dot at beginning:
	check_ok("", "..");
	check_ok("", "../");
	check_ok("alpha", "../alpha");
	check_ok("/alpha", "/../alpha");

	// single dot in middle:
	check_ok("alpha/bravo", "alpha/./bravo");
	check_ok("alpha/bravo", "alpha/././bravo");

	// double dot in middle:
	check_ok("bravo", "alpha/../bravo");
	check_ok("bravo", "alpha/../alpha/../bravo");

	// single dot at end:
	check_ok("alpha/bravo/", "alpha/bravo/.");
	check_ok("alpha/bravo/", "alpha/bravo/./");

	// double dot at end:
	check_ok("alpha/", "alpha/bravo/..");
	check_ok("alpha/", "alpha/bravo/../");

	// multiple single dots:
	check_ok("alpha/bravo", "alpha/././././bravo");

	// multiple double dots:
	check_ok("delta", "alpha/../bravo/../charlie/../delta");
	check_ok("alpha/", "alpha/bravo/charlie/delta/../../..");
	check_ok("alpha/", "alpha/bravo/charlie/delta/../../../");

	// examples from RFC #3986, section 5.2.4 ("Remove Dot Segments")
	check_ok("/a/g", "/a/b/c/./../../g");
	check_ok("mid/6", "mid/content=5/../6");

	return 0;
}

