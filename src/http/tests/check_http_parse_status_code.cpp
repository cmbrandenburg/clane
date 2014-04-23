// vim: set noet:

#include "../clane_http_parse.hpp"
#include "../../clane_check.hpp"

using namespace clane;

#define check_ok(in, exp_scode) \
	do { \
		std::string s(in); \
		http::status_code scode{}; \
		check(http::parse_status_code(&scode, s)); \
		check(scode == exp_scode); \
	} while (false)

#define check_nok(in) \
	do { \
		std::string s(in); \
		http::status_code scode{}; \
		check(!http::parse_status_code(&scode, s)); \
	} while (false)

int main() {
	check_ok("200", http::status_code::ok);
	check_ok("404", http::status_code::not_found);
	check_nok("");
	check_nok("200 ");
	check_nok(" 200");
	check_nok("200a");
	check_nok("-200");
	check_nok("1234");
}

