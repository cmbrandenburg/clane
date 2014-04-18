// vim: set noet:

#include "clane_check.hpp"
#include "../clane_http_parse.hpp"
#include <cstring>

using namespace clane;

void check_ok(char const *content, char const *exp_method, char const *exp_uri, int exp_major, int exp_minor) {

	static char const *const empty = "";
	std::string const s = std::string(content) + "extra";
	http::v1x_request_line_incparser pars;
	std::error_code e;

	// single pass:
	pars.reset();
	check(std::strlen(content) == pars.parse_some(s.data(), s.data()+s.size()));
	check(!pars);
	check(pars.method() == exp_method);
	check(pars.uri().string() == exp_uri);
	check(pars.major_version() == exp_major);
	check(pars.minor_version() == exp_minor);

	// byte-by-byte:
	pars.reset();
	check(pars);
	for (size_t i = 0; i < std::strlen(content)-1; ++i) {
		check(0 == pars.parse_some(empty, empty));
		check(pars);
		check(1 == pars.parse_some(s.data()+i, s.data()+i+1));
		check(pars);
	}
	check(0 == pars.parse_some(empty, empty));
	check(pars);
	check(1 == pars.parse_some(s.data()+std::strlen(content)-1, s.data()+std::strlen(content)));
	check(!pars);
	check(pars.method() == exp_method);
	check(pars.uri().string() == exp_uri);
	check(pars.major_version() == exp_major);
	check(pars.minor_version() == exp_minor);
}

void check_nok(size_t len_limit, char const *ok, char const *bad, http::status_code exp_error_code) {

	static char const *const empty = "";
	std::string full(ok);
 	full.append(bad);
	http::v1x_request_line_incparser pars;
	std::error_code e;

	// single pass:
	pars.reset();
	pars.set_length_limit(len_limit);
	check(pars.error == pars.parse_some(full.data(), full.data()+full.size()));
	check(pars.status() == exp_error_code);
	check(!pars);

	// byte-by-byte:
	e.clear();
	pars.reset();
	for (size_t i = 0; i < std::strlen(ok); ++i) {
		check(0 == pars.parse_some(empty, empty));
		check(pars);
		check(1 == pars.parse_some(ok+i, ok+i+1));
		check(pars);
	}
	check(0 == pars.parse_some(empty, empty));
	check(pars);
	check(pars.error == pars.parse_some(bad, bad+1));
	check(pars.status() == exp_error_code);
	check(!pars);
}

int main() {

	check_ok("GET / HTTP/1.1\r\n", "GET", "/", 1, 1);
	check_ok("GET /foo?bar HTTP/1.1\r\n", "GET", "/foo?bar", 1, 1);
	check_ok("GET /foo?bar HTTP/1.0\r\n", "GET", "/foo?bar", 1, 0);

	// not-OK: empty line
	check_nok(0, "", "\r\n", http::status_code::bad_request);

	// not-OK: missing version
	check_nok(0, "GET /foo ", "\r\n", http::status_code::bad_request);
	check_nok(0, "GET /foo", "\r\n", http::status_code::bad_request);

	// not-OK: missing URI
	check_nok(0, "GET ", "\r\n", http::status_code::bad_request);
	check_nok(0, "GET", "\r\n", http::status_code::bad_request);

	// not-OK: invalid method
	check_nok(0, "GE\tT", " /foo HTTP/1.1\r\n", http::status_code::bad_request);

	// not-OK: invalid URI
	check_nok(0, "GET /fo\to", " HTTP/1.1\r\n", http::status_code::bad_request);

	// not-OK: invalid version
	check_nok(0, "GET /foo blah/1.1", "\r\n", http::status_code::bad_request);
	check_nok(0, "GET /foo http/1.1", "\r\n", http::status_code::bad_request);
	check_nok(0, "GET /foo HTTP/a.1", "\r\n", http::status_code::bad_request);
	check_nok(0, "GET /foo HTTP/1.a", "\r\n", http::status_code::bad_request);
	check_nok(0, "GET /foo HTTP/-1.1", "\r\n", http::status_code::bad_request);
	check_nok(0, "GET /foo HTTP/1.-1", "\r\n", http::status_code::bad_request);
	check_nok(0, "GET /foo HTTP/1.1 ", "\r\n", http::status_code::bad_request);
}

