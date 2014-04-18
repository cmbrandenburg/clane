// vim: set noet:

#include "clane_check.hpp"
#include "../clane_http_parse.hpp"

using namespace clane;

#define check_ok(in, exp_major, exp_minor) \
	do { \
		std::string s(in); \
		int major{}, minor{}; \
		check(http::parse_version(&major, &minor, s)); \
		check(major == exp_major); \
		check(minor == exp_minor); \
	} while (false)

#define check_nok(in) \
	do { \
		std::string s(in); \
		int major{}, minor{}; \
		check(!http::parse_version(&major, &minor, s)); \
	} while (false)

int main() {
	check_ok("HTTP/1.0", 1, 0);
	check_ok("HTTP/1.1", 1, 1);
	check_nok("");
	check_nok("HTTP");
	check_nok("HTTP/");
	check_nok("HTTP/1");
	check_nok("HTTP/1.");
	check_nok("HTTP/1.1 ");
	check_nok(" HTTP/1.1");
	check_nok("HTTP/-1.1");
	check_nok("HTTP/1.-1");
}

