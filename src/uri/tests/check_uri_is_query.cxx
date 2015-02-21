// vim: set noet:

#include "check/check.h"
#include "../clane_uri.hxx"
#include <cstring>

#define check_ok(in) check(clane::uri::is_query(in, in+strlen(in)))
#define check_nok(in) check(!clane::uri::is_query(in, in+strlen(in)))

int main() {
	check_ok("");
	check_ok("alpha");
	check_ok("alpha?bravo=charlie");
	check_ok("alpha%20bravo");
	check_nok("alpha#");
	check_nok("alpha%XXbravo");
}

