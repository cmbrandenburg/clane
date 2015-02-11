// vim: set noet:

#include "check/check.h"

#define check_nok(u) \
	do { \
		std::error_code e; \
		u.validate(e); \
		check(e); \
	} while (false)

int main() {

#if 0
	uri::uri u;

	// authority with relative path:
	u.clear();
	u.host = "alpha";
	u.path = "bravo";
	check_nok(u);

	// no authority
	u.clear();
	u.path = "//alpha";
	check_nok(u);

	// no scheme, no authority, first path segment with colon:
	u.clear();
	u.path = "alpha:bravo";
	check_nok(u);
#endif
	return 1;
}

