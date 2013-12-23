// vim: set noet:

#include "../http_parse.h"
#include "../../check/check.h"
#include <cstring>

using namespace clane;

class nop_parser: public http::parser {
public:
	using http::parser::increase_length;
	using http::parser::set_error;
};

int main() {

	nop_parser p;

	// default OK state:
	check(p);
	check(0 == p.parse_length());

	check(p.increase_length(1234));
	check(1234 == p.parse_length());

	// setting error state:
	p.set_error(http::status_code::not_found, "my error message");
	check(!p);
	check(p.error_code() == http::status_code::not_found);
	check(!strcmp(p.what(), "my error message"));

	// reset:
	p.reset();
	check(p);
	check(0 == p.parse_length());

	// length limits:
	p.set_length_limit(100);
	check(p.increase_length(100));
	p.reset();
	check(!p.increase_length(101));
	p.reset();
	check(p.increase_length(99));
	check(p.increase_length(1));
	check(!p.increase_length(1));

	return 0;
}

