// vim: set noet:

#include "../../check/check.h"
#include "../http_consume.hpp"

using namespace clane;

static void check_ok(char const *s, http::status_code exp_stat) {
	std::string in(s);
	http::status_code stat{};
	check(http::parse_status_code(&stat, in));
	check(stat == exp_stat);
}

static void check_nok(char const *s) {
	std::string in(s);
	http::status_code stat{};
	check(!http::parse_status_code(&stat, in));
}

int main() {

	// valid:
	check_ok("200", http::status_code::ok);
	check_ok("404", http::status_code::not_found);

	// invalid:
	check_nok("");
	check_nok("200 ");
	check_nok(" 200");
	check_nok("200a");
	check_nok("-200");
	check_nok("1234");
}

