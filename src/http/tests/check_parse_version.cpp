// vim: set noet:

#include "../../check/check.h"
#include "../http_consume.hpp"

using namespace clane;

static void check_ok(char const *s, int exp_major, int exp_minor) {
	std::string in(s);
	int major{}, minor{};
	check(http::parse_version(&major, &minor, in));
	check(major == exp_major);
	check(minor == exp_minor);
}

static void check_nok(char const *s) {
	std::string in(s);
	int major{}, minor{};
	check(!http::parse_version(&major, &minor, in));
}

int main() {

	// valid:
	check_ok("HTTP/1.0", 1, 0);
	check_ok("HTTP/1.1", 1, 1);

	// invalid:
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

