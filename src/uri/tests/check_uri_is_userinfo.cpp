// vim: set noet:

#include "../../clane_check.hpp"
#include "../clane_uri.hpp"
#include <cstring>

using namespace clane;

#define check_ok(in) check(uri::is_user_info(in, in+strlen(in)))
#define check_nok(in) check(!uri::is_user_info(in, in+strlen(in)))

int main() {
	check_ok("john_doe123");
	check_ok(":'!"); // colon and sub-delimiters
	check_ok("john%20doe");
	check_nok("john/doe"); // general delimiters
	check_nok("john%ggdoe");
}

