// vim: set noet:

#include "../clane_http_parse.hpp"
#include "../../clane_check.hpp"
#include <cstring>

using namespace clane;

void check_ok(char const *content, size_t exp_size) {

	static char const *const empty = "";
	std::string const s = std::string(content) + "extra";
	http::v1x_chunk_line_incparser pars;

	// single pass:
	pars.reset();
	check(std::strlen(content) == pars.parse_some(s.data(), s.data()+s.size()));
	check(!pars);
	check(pars.chunk_size() == exp_size);

	// byte-by-byte:
	pars.reset();
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
	check(pars.chunk_size() == exp_size);
}

void check_nok(size_t len_limit, char const *ok, char const *bad, http::status_code exp_error_code) {

	static char const *const empty = "";
	std::string full(ok);
	full.append(bad);
	http::v1x_chunk_line_incparser pars;

	// single pass:
	pars.reset();
	pars.set_length_limit(len_limit);
	check(pars.error == pars.parse_some(full.data(), full.data()+full.size()));
	check(pars.status() == exp_error_code);
	check(!pars);

	// byte-by-byte:
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

	check_ok("0\r\n", 0);
	check_ok("0\n", 0);
	check_ok("123\r\n", 0x123);
	check_ok("123\n", 0x123);
	check_ok("ABC\r\n", 0xabc);
	check_ok("ABC\n", 0xabc);
	check_ok("abc\r\n", 0xabc);
	check_ok("abc\n", 0xabc);
	check_ok("1bC\r\n", 0x1bc);

	check_nok(0, "", "\r\n", http::status_code::bad_request);
	check_nok(0, "12", "invalid34\r\n", http::status_code::bad_request);
	check_nok(0, "1234", " \r\n", http::status_code::bad_request);
	check_nok(0, "12", " 34\r\n", http::status_code::bad_request);
}

