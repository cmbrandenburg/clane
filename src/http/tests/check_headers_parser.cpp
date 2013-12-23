// vim: set noet:

#include "../http_parse.h"
#include "../../check/check.h"
#include <cstring>

using namespace clane;

void check_ok(char const *content, http::header_map const &exp_hdrs) {

	std::string const s = std::string(content) + "extra";

	// single pass:
	http::headers_parser p;
	check(p.parse(s.data(), s.size()));
	check(strlen(content) == p.parse_length());
	check(exp_hdrs == p.headers());

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
	check(exp_hdrs == p.headers());
}

int main() {

	http::headers_parser p;

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

	return 0;
}

