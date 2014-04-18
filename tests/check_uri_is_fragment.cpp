// vim: set noet:

#include "clane_check.hpp"
#include "../clane_uri.hpp"
#include <cstring>

using namespace clane;

#define check_ok(in) check(uri::is_fragment(in, in+strlen(in)))
#define check_nok(in) check(!uri::is_fragment(in, in+strlen(in)))

int main() {
	check_ok("");
	check_ok("alpha");
	check_ok("alpha?bravo=charlie");
	check_ok("alpha%20bravo");
	check_ok("alpha/@:bravo");
	check_nok("alpha#");
	check_nok("alpha%XXbravo");
}

