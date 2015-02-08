// vim: set noet:

#include "clane_check.hpp"
#include "../clane_http_parse.hpp"
#include <cstring>

using namespace clane;

void check_ok(http::v1x_body_incparser::length_type len_type, size_t len, char const *content, std::string const &exp) {

	static char const *const empty = "";
	std::string got;
	http::v1x_body_incparser pars;

	// as few passes as possible, given that the parser returns incomplete when
	// the body is chunked:
	pars.reset(len_type, len);
	size_t offset = 0;
	do {
		size_t stat = pars.parse_some(content+offset, content+std::strlen(content));
		check(pars.error != stat);
		got.append(content+offset+pars.offset(), pars.size());
		offset += stat;
		if (len_type != http::v1x_body_incparser::chunked) {
			check(stat == std::strlen(content));
			break;
		}
	} while (pars);
	check(len_type == http::v1x_body_incparser::infinite || !pars);
	check(got == exp);

	// byte-by-byte:
	pars.reset(len_type, len);
	got.clear();
	check(0 == pars.parse_some(empty, empty));
	got.append(empty+pars.offset(), pars.size());
	if (!std::strlen(content)) {
		check(!pars);
	} else {
		check(pars);
		for (size_t i = 0; i < std::strlen(content)-1; ++i) {
			check(1 == pars.parse_some(content+i, content+i+1));
			got.append(content+i+pars.offset(), pars.size());
			check(pars);
			check(0 == pars.parse_some(empty, empty));
			got.append(content+i+pars.offset(), pars.size());
			check(pars);
		}
		check(1 == pars.parse_some(content+std::strlen(content)-1, content+std::strlen(content)));
		got.append(content+std::strlen(content)-1+pars.offset(), pars.size());
		check(len_type == http::v1x_body_incparser::infinite || !pars);
	}
	check(got == exp);
}

void check_nok(char const *ok, char const *bad, http::status_code exp_error_code) {

	static char const *const empty = "";
	std::string full(ok);
	full.append(bad);
	http::v1x_body_incparser pars;

	// as few passes as possible:
	pars.reset(http::v1x_body_incparser::chunked, 0);
	size_t offset = 0;
	while (true) {
		size_t stat = pars.parse_some(full.data()+offset, full.data()+full.size());
		if (pars.error == stat) {
			check(pars.status() == exp_error_code);
			break;
		}
		offset += stat;
	}
	check(!pars);

	// byte-by-byte:
	pars.reset(http::v1x_body_incparser::chunked, 0);
	for (size_t i = 0; i < std::strlen(ok); ++i) {
		check(0 == pars.parse_some(empty, empty));
		check(pars);
		check(1 == pars.parse_some(ok+i, ok+i+1));
		check(pars);
	}
	check(0 == pars.parse_some(bad, bad));
	check(pars);
	check(pars.error == pars.parse_some(bad, bad+1));
	check(pars.status() == exp_error_code);
	check(!pars);
}

int main() {

	// fixed:
	check_ok(http::v1x_body_incparser::fixed, 0, "", "");
	check_ok(http::v1x_body_incparser::fixed, 13, "Hello, world.", "Hello, world.");

	// infinite:
	check_ok(http::v1x_body_incparser::infinite, 0, "Hello, world.", "Hello, world.");

	// one chunk, no error:
	check_ok(http::v1x_body_incparser::chunked, 0, "d\r\nHello, world.\r\n0\r\n", "Hello, world.");
	check_ok(http::v1x_body_incparser::chunked, 0, "d\nHello, world.\r\n0\r\n", "Hello, world.");
	check_ok(http::v1x_body_incparser::chunked, 0, "d\r\nHello, world.\n0\r\n", "Hello, world.");
	check_ok(http::v1x_body_incparser::chunked, 0, "d\r\nHello, world.\r\n0\n", "Hello, world.");
	check_ok(http::v1x_body_incparser::chunked, 0, "d\nHello, world.\n0\n", "Hello, world.");

	// two chunks, no error:
	check_ok(http::v1x_body_incparser::chunked, 0, "7\r\nHello, \r\n6\r\nworld.\r\n0\r\n", "Hello, world.");
	check_ok(http::v1x_body_incparser::chunked, 0, "7\nHello, \r\n6\r\nworld.\r\n0\r\n", "Hello, world.");
	check_ok(http::v1x_body_incparser::chunked, 0, "7\r\nHello, \n6\r\nworld.\r\n0\r\n", "Hello, world.");
	check_ok(http::v1x_body_incparser::chunked, 0, "7\r\nHello, \r\n6\nworld.\r\n0\r\n", "Hello, world.");
	check_ok(http::v1x_body_incparser::chunked, 0, "7\r\nHello, \r\n6\r\nworld.\n0\r\n", "Hello, world.");
	check_ok(http::v1x_body_incparser::chunked, 0, "7\r\nHello, \r\n6\r\nworld.\r\n0\n", "Hello, world.");

	// multi-digit chunk length:
	check_ok(http::v1x_body_incparser::chunked, 0, "1a\r\nHello, world. How are you?\r\n0\r\n", "Hello, world. How are you?");

	// chunked, error:
	check_nok("", "\r\nHello, world.\r\n0\r\n", http::status_code::bad_request); // missing first chunk size
	check_nok("d\r\nHello, world.", "0\r\n0\r\n", http::status_code::bad_request); // expected \r or \n, got 0
	check_nok("d\r\nHello, world.\r", "\r\n0\r\n", http::status_code::bad_request); // expected \n, got \r
	check_nok("d\r\nHello, world.\r\n0", "invalid\r\n", http::status_code::bad_request); // expected \r or \n, got i
	check_nok("d\r\nHello, world.\r\n0\r", "invalid\n", http::status_code::bad_request); // expected \r or \n, got i

}

