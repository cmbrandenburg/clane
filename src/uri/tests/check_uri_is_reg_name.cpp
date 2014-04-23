// vim: set noet:

#include "../../clane_check.hpp"
#include "../clane_uri.hpp"
#include <cstring>

using namespace clane;

#define check_ok(in) check(uri::is_reg_name(in, in+strlen(in)))
#define check_nok(in) check(!uri::is_reg_name(in, in+strlen(in)))

int main() {
	check_ok("");
	check_ok("alpha.bravo+charlie");
	check_nok("alpha/bravo");
}

