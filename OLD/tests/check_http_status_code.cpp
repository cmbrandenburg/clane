// vim: set noet:

#include "clane_check.hpp"
#include "clane_http_message.hpp"

using namespace clane;

#define check_ok(scode, exp) \
	do { \
		check(std::string(what(scode)) == exp); \
	} while (false)

int main() {
	check_ok(http::status_code::ok, "OK");
	check_ok(http::status_code::not_found, "Not found");
	check_ok(static_cast<http::status_code>(0), "");
}

