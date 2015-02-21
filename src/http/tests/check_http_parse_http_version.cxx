#include "check/check.h"
#include "../clane_http_message.hxx"
#include <cstring>

#define check_ok(in, exp_major, exp_minor) \
	do { \
		unsigned major, minor; \
		check(clane::http::parse_http_version(in, in+std::strlen(in), major, minor)); \
		check(major == exp_major); \
		check(minor == exp_minor); \
	} while (false)

#define check_nok(in) \
	do { \
		unsigned major, minor; \
		check(!clane::http::parse_http_version(in, in+std::strlen(in), major, minor)); \
	} while (false)

int main() {
	check_ok("HTTP/1.0", 1, 0);
	check_ok("HTTP/1.1", 1, 1);
	check_ok("HTTP/1234.5678", 1234, 5678);
	check_nok("");
	check_nok("HTTP/");
	check_nok("HTTP/1");
	check_nok("HTTP/1.");
	check_nok("HTTP/12.");
	check_nok(" HTTP/1.1");
	check_nok(" HTTP/1.1 ");
	check_nok("HTTP1.1");
}

