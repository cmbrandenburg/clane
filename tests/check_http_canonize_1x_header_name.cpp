// vim: set noet:

#include "clane_check.hpp"
#include "../clane_http_message.hpp"

using namespace clane;

#define check_ok(in, exp) \
	do { \
		std::string s(in); \
		http::canonize_1x_header_name(&s[0], &s[0]+s.size()); \
		check(s == exp); \
	} while (false)

int main() {
	check_ok("", "");
	check_ok("Content-Length", "Content-Length");
	check_ok("content-length", "Content-Length");
	check_ok("CONTENT-LENGTH", "Content-Length");
}

