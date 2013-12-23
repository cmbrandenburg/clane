// vim: set noet:

#include "../http_parse.h"
#include "../../check/check.h"
#include <cstring>

using namespace clane;

void check_ok(char const *content, char const *exp_method, char const *exp_uri, int exp_major, int exp_minor) {

	std::string const s = std::string(content) + "extra";

	// single pass:
	http::request_line_parser p;
	check(p.parse(s.data(), s.size()));
	check(strlen(content) == p.parse_length());
	check(p.method() == exp_method);
	check(p.uri().to_string() == exp_uri);
	check(p.major_version() == exp_major);
	check(p.minor_version() == exp_minor);

	// byte-by-byte:
	p.reset();
	for (size_t i = 0; i < strlen(content)-1; ++i) {
		check(!p.parse("", 0));
		check(p);
		check(!p.parse(s.data()+i, 1));
		check(p);
	}
	check(!p.parse("", 0));
	check(p);
	check(p.parse(s.data()+strlen(content)-1, 1));
	check(strlen(content) == p.parse_length());
	check(p.method() == exp_method);
	check(p.uri().to_string() == exp_uri);
	check(p.major_version() == exp_major);
	check(p.minor_version() == exp_minor);
}

void check_nok(size_t len_limit, char const *s, http::status_code exp_error_code) {
	// single pass:
	http::request_line_parser p;
	p.set_length_limit(len_limit);
	check(!p.parse(s, strlen(s)));
	check(!p);
	check(exp_error_code == p.error_code());
}

int main() {

	check_ok("GET / HTTP/1.1\r\n", "GET", "/", 1, 1);

	return 0;
}

