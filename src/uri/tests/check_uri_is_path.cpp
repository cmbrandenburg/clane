// vim: set noet:

#include "check/check.h"
#include "../clane_uri_parse.hpp"
#include <cstring>

#define check_ok(in) check(clane::uri::is_path(in, in+strlen(in)))
#define check_nok(in) check(!clane::uri::is_path(in, in+strlen(in)))

int main() {
	check_ok("");
	check_ok("alpha.bravo+charlie");
	check_ok("alpha/bravo");
	check_ok("alpha%20bravo");
	check_nok("alpha/bravo?");
	check_nok("alpha/bravo#");
	check_nok("alpha%XXbravo");
}

