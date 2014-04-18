// vim: set noet:

#include "clane_check.hpp"
#include "../clane_http_parse.hpp"
#include <cstring>

using namespace clane;

void check_ok(char const *content, http::header_map const &exp_hdrs, size_t len_limit = 0) {

	static char const *const empty = "";
	std::string const s = std::string(content) + "extra";
	http::v1x_headers_incparser pars;

	// single pass:
	pars.reset();
	pars.set_length_limit(len_limit);
	check(std::strlen(content) == pars.parse_some(s.data(), s.data()+s.size()));
	check(!pars);
	check(pars.headers() == exp_hdrs);

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
	check(pars.headers() == exp_hdrs);
}

void check_nok(size_t len_limit, char const *ok, char const *bad, http::status_code exp_error_code) {

	static char const *const empty = "";
	http::header_map got_hdrs;
	std::string full(ok);
 	full.append(bad);
	http::v1x_headers_incparser pars;

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

	check_ok("\r\n", http::header_map({}));
	check_ok("alpha: bravo\r\n\r\n", http::header_map({http::header("alpha", "bravo")}));
	check_ok("alpha: bravo\r\ncharlie: delta\r\n\r\n", http::header_map({
		http::header("alpha", "bravo"),
		http::header("charlie", "delta")}));

	// newline without carriage return:
	check_ok("alpha: bravo\n\n", http::header_map({http::header("alpha", "bravo")}));
	check_ok("alpha: bravo\ncharlie: delta\n\n", http::header_map({
		http::header("alpha", "bravo"),
		http::header("charlie", "delta")}));

	// funky whitespace:
	check_ok("alpha:bravo\r\n\r\n", http::header_map({http::header("alpha", "bravo")}));
	check_ok("alpha   :bravo\r\n\r\n", http::header_map({http::header("alpha", "bravo")}));
	check_ok("alpha\t\t\t:bravo\r\n\r\n", http::header_map({http::header("alpha", "bravo")}));
	check_ok("alpha \t :bravo\r\n\r\n", http::header_map({http::header("alpha", "bravo")}));
	check_ok("alpha:    bravo\r\n\r\n", http::header_map({http::header("alpha", "bravo")}));
	check_ok("alpha:\t\t\t\tbravo\r\n\r\n", http::header_map({http::header("alpha", "bravo")}));
	check_ok("alpha: \t \t bravo\r\n\r\n", http::header_map({http::header("alpha", "bravo")}));
	check_ok("alpha: bravo   \r\n\r\n", http::header_map({http::header("alpha", "bravo")}));
	check_ok("alpha: bravo\t\t\t\r\n\r\n", http::header_map({http::header("alpha", "bravo")}));
	check_ok("alpha: bravo \t \r\n\r\n", http::header_map({http::header("alpha", "bravo")}));

	// linear whitespace:
	check_ok("alpha: bravo\r\n charlie delta\r\n\r\n", http::header_map({
		http::header("alpha", "bravo charlie delta")}));
	check_ok("alpha: bravo\r\n\tcharlie delta\r\n\r\n", http::header_map({
		http::header("alpha", "bravo charlie delta")}));

	// not-OK: missing colon between name and value
	check_nok(0, "alpha bravo", "\r\n\r\n", http::status_code::bad_request);
	check_nok(0, "alpha bravo", "\r\ncharlie: delta\r\n", http::status_code::bad_request);

	// not-OK: invalid header name
	check_nok(0, "al\tpha", ": bravo\r\n\r\n", http::status_code::bad_request);
	check_nok(0, "alpha: bravo\r\ncha\trlie", ": delta\r\n\r\n", http::status_code::bad_request);

	// not-OK: invalid header value
	check_nok(0, "alpha: bra\r", "vo\r\n\r\n", http::status_code::bad_request);

	// not-OK: line too long
	check_nok(3, "alp", "ha: bravo\r\n\r\n", http::status_code::bad_request);
	check_nok(6, "alpha:", " bravo\r\n\r\n", http::status_code::bad_request);
	check_nok(7, "alpha: ", "bravo\r\n\r\n", http::status_code::bad_request);
	check_nok(12, "alpha: bravo", "\r\n\r\n", http::status_code::bad_request);
	check_nok(13, "alpha: bravo\r", "\n\r\n", http::status_code::bad_request);
	check_nok(14, "alpha: bravo\r\n", "\r\n", http::status_code::bad_request);
	check_nok(15, "alpha: bravo\r\n\r", "\n", http::status_code::bad_request);
	check_ok("alpha: bravo\r\n\r\n", http::header_map({http::header("alpha", "bravo")}), 16);
}

