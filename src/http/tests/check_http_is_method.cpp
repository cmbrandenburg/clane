#include "check/check.h"
#include "../clane_http_message.hpp"
#include <cstring>

#define check_ok(in) \
	do { \
		check(clane::http::is_method(in, in+std::strlen(in))); \
	} while (false)

#define check_nok(in) \
	do { \
		check(!clane::http::is_method(in, in+std::strlen(in))); \
	} while (false)

int main() {
	check_ok("GET");
	check_ok("PUT");
	check_ok("ok_method");
	check_nok("bad\nmethod");
	check_nok("bad:method");
}

