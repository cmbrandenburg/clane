// vim: set noet:

#include "../uri.cpp"
#include "../../check/check.h"

#define check(exp_result, s) \
	do { \
		check_(__FILE__, __LINE__, exp_result, s); \
	} while (false)

static void check_(char const *sname, int sline, char const *exp_result, char const *s);

void check_(char const *sname, int sline, char const *exp_result, char const *s) {
	std::string got_result(s);
	clane::uri::normalize_path(got_result);
	clane::check::check_eq_(sname, sline, exp_result, got_result);
}

int main() {

	check("", "");
	check("/", "/");
	check("alpha", "alpha");
	check("alpha/bravo/charlie", "alpha/bravo/charlie");

	// trailing slash is preserved:
	check("alpha/bravo/charlie/", "alpha/bravo/charlie/");

	// absolute path:
	check("/alpha/bravo/charlie", "/alpha/bravo/charlie");
	check("/alpha/bravo/charlie/", "/alpha/bravo/charlie/");

	// single dot at beginning:
	check("", ".");
	check("", "./");
	check("alpha", "./alpha");
	check("/alpha", "/./alpha");

	// double dot at beginning:
	check("", "..");
	check("", "../");
	check("alpha", "../alpha");
	check("/alpha", "/../alpha");

	// single dot in middle:
	check("alpha/bravo", "alpha/./bravo");
	check("alpha/bravo", "alpha/././bravo");

	// double dot in middle:
	check("bravo", "alpha/../bravo");
	check("bravo", "alpha/../alpha/../bravo");

	// single dot at end:
	check("alpha/bravo/", "alpha/bravo/.");
	check("alpha/bravo/", "alpha/bravo/./");

	// double dot at end:
	check("alpha/", "alpha/bravo/..");
	check("alpha/", "alpha/bravo/../");

	// multiple single dots:
	check("alpha/bravo", "alpha/././././bravo");

	// multiple double dots:
	check("delta", "alpha/../bravo/../charlie/../delta");
	check("alpha/", "alpha/bravo/charlie/delta/../../..");
	check("alpha/", "alpha/bravo/charlie/delta/../../../");

	// examples from RFC #3986, section 5.2.4 ("Remove Dot Segments")
	check("/a/g", "/a/b/c/./../../g");
	check("mid/6", "mid/content=5/../6");

	return 0;
}

