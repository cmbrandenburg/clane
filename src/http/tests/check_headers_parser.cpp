// vim: set noet:

#include "../http_parse.h"
#include "../../check/check.h"
#include <cstring>

using namespace clane;

void check_ok(char const *content, http::header_map const &exp_hdrs) {

	std::string const s = std::string(content) + "extra";
	http::header_map got_hdrs;

	// single pass:
	http::headers_parser p(got_hdrs);
	check(p.parse(s.data(), s.size()));
	check(strlen(content) == p.parse_length());
	check(exp_hdrs == got_hdrs);

	// byte-by-byte:
	got_hdrs.clear();
	p.reset(got_hdrs);
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
	check(exp_hdrs == got_hdrs);
}

void check_nok(size_t len_limit, char const *s, http::status_code exp_error_code) {
	http::header_map got_hdrs;
	// single pass:
	http::headers_parser p(got_hdrs);
	p.set_length_limit(len_limit);
	check(!p.parse(s, strlen(s)));
	check(!p);
	check(exp_error_code == p.error_code());
}

int main() {

	check_ok("\r\n", http::header_map({}));
	check_ok("alpha: bravo\r\n\r\n", http::header_map({http::header_map::value_type("alpha", "bravo")}));
	check_ok("alpha: bravo\r\ncharlie: delta\r\n\r\n", http::header_map({
		http::header_map::value_type("alpha", "bravo"),
		http::header_map::value_type("charlie", "delta")}));

	// newline without carriage return:
	check_ok("alpha: bravo\n\n", http::header_map({http::header_map::value_type("alpha", "bravo")}));
	check_ok("alpha: bravo\ncharlie: delta\n\n", http::header_map({
		http::header_map::value_type("alpha", "bravo"),
		http::header_map::value_type("charlie", "delta")}));

	// funky whitespace:
	check_ok("alpha:bravo\r\n\r\n", http::header_map({http::header_map::value_type("alpha", "bravo")}));
	check_ok("alpha   :bravo\r\n\r\n", http::header_map({http::header_map::value_type("alpha", "bravo")}));
	check_ok("alpha\t\t\t:bravo\r\n\r\n", http::header_map({http::header_map::value_type("alpha", "bravo")}));
	check_ok("alpha \t :bravo\r\n\r\n", http::header_map({http::header_map::value_type("alpha", "bravo")}));
	check_ok("alpha:    bravo\r\n\r\n", http::header_map({http::header_map::value_type("alpha", "bravo")}));
	check_ok("alpha:\t\t\t\tbravo\r\n\r\n", http::header_map({http::header_map::value_type("alpha", "bravo")}));
	check_ok("alpha: \t \t bravo\r\n\r\n", http::header_map({http::header_map::value_type("alpha", "bravo")}));

	// linear whitespace:
	check_ok("alpha: bravo\r\n charlie delta\r\n\r\n", http::header_map({
		http::header_map::value_type("alpha", "bravo charlie delta")}));
	check_ok("alpha: bravo\r\n\tcharlie delta\r\n\r\n", http::header_map({
		http::header_map::value_type("alpha", "bravo charlie delta")}));

	// not-OK: missing colon between name and value
	check_nok(0, "alpha bravo\r\n\r\n", http::status_code::bad_request);

	// not-OK: invalid header name
	check_nok(0, "al\tpha: bravo\r\n\r\n", http::status_code::bad_request);
	check_nok(0, "alpha: bravo\r\ncha\trlie: delta\r\n\r\n", http::status_code::bad_request);

	// not-OK: invalid header value
	check_nok(0, "alpha: bra\rvo\r\n\r\n", http::status_code::bad_request);

	// not-OK: line too long
	check_nok(3, "alpha: bravo\r\n\r\n", http::status_code::bad_request);
	check_nok(7, "alpha: bravo\r\n\r\n", http::status_code::bad_request);
	check_nok(12, "alpha: bravo\r\n\r\n", http::status_code::bad_request);
	check_nok(13, "alpha: bravo\r\n\r\n", http::status_code::bad_request);

	return 0;
}

