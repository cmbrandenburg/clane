// vim: set noet:

#include "../clane_http_parse.hpp"
#include "../../clane_check.hpp"
#include <cstring>

using namespace clane;

void check_ok(char const *content, int exp_major, int exp_minor, http::status_code exp_stat, char const *exp_reason) {

	static char const *const empty = "";
	std::string const s = std::string(content) + "extra";
	http::v1x_status_line_incparser pars;

	// single pass:
	pars.reset();
	check(std::strlen(content) == pars.parse_some(s.data(), s.data()+s.size()));
	check(!pars);
	check(pars.major_version() == exp_major);
	check(pars.minor_version() == exp_minor);
	check(pars.status() == exp_stat);
	check(pars.reason() == exp_reason);

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
	check(pars.major_version() == exp_major);
	check(pars.minor_version() == exp_minor);
	check(pars.status() == exp_stat);
	check(pars.reason() == exp_reason);
}

void check_nok(size_t len_limit, char const *ok, char const *bad) {

	static char const *const empty = "";
	std::string full(ok);
 	full.append(bad);
	http::v1x_status_line_incparser pars;

	// single pass:
	pars.reset();
	pars.set_length_limit(len_limit);
	check(pars.error == pars.parse_some(full.data(), full.data()+full.size()));
	check(!pars);

	// byte-by-byte:
	pars.reset();
	check(pars);
	for (size_t i = 0; i < std::strlen(ok); ++i) {
		check(0 == pars.parse_some(empty, empty));
		check(pars);
		check(1 == pars.parse_some(ok+i, ok+i+1));
		check(pars);
	}
	check(0 == pars.parse_some(empty, empty));
	check(pars);
	check(pars.error == pars.parse_some(bad, bad+1));
	check(!pars);
}

int main() {

	check_ok("HTTP/1.1 200 OK\r\n", 1, 1, http::status_code::ok, "OK");
	check_ok("HTTP/1.1 200 OK\n", 1, 1, http::status_code::ok, "OK");
	check_ok("HTTP/1.1 200 \r\n", 1, 1, http::status_code::ok, "");

	// not-OK: empty line
	check_nok(0, "", "\r\n");

	// not-OK: missing reason phrase
	check_nok(0, "HTTP/1.1 200", "\r\n");

	// not-OK: missing status code
	check_nok(0, "HTTP/1.1 ", "\r\n");
	check_nok(0, "HTTP/1.1", "\r\n");

	// not-OK: invalid version
	check_nok(0, "HTTP/1.", " 200 OK\r\n");
}

