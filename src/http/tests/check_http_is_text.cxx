#include "check/check.h"
#include "../clane_http_message.hxx"
#include <cstring>

#define check_ok(in) \
	do { \
		check(clane::http::is_text(in, in+std::strlen(in))); \
	} while (false)

#define check_nok(in) \
	do { \
		check(!clane::http::is_text(in, in+std::strlen(in))); \
	} while (false)

int main() {
	check_ok("alpha bravo\tcharlie<>{}[]");
	check_nok("\x01\n\x7f");
}

