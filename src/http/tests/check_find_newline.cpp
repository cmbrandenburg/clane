// vim: set noet:

#include "../../check/check.h"
#include "../http_consume.hpp"
#include <cstring>

using namespace clane;

int main() {
	char const *p;

	p = "";
	char const *got = http::find_newline(p, strlen(p));
	check(p + strlen(p) == got);

	p = "alpha bravo\tcharlie";
	got = http::find_newline(p, strlen(p));
	check(p + strlen(p) == got);

	p = "alpha\nbravo";
	got = http::find_newline(p, strlen(p));
	check(p + 5 == got);

	p = "alpha\nbravo\n";
	got = http::find_newline(p, strlen(p));
	check(p + 5 == got);

	p = "alpha\r\nbravo";
	got = http::find_newline(p, strlen(p));
	check(p + 5 == got);

	p = "alpha\r\nbravo\r\n";
	got = http::find_newline(p, strlen(p));
	check(p + 5 == got);

	p = "alpha\rbravo\r\n";
	got = http::find_newline(p, strlen(p));
	check(p + 11 == got);

	p = "alpha\r";
	got = http::find_newline(p, strlen(p));
	check(p + 5 == got);

	p = "\r\n";
	got = http::find_newline(p, strlen(p));
	check(p == got);

	std::string s("alpha");
	s.push_back(0);
	s.append("bravo\r\n");
	p = s.data();
	got = http::find_newline(p, s.size());
	check(p + 11 == got);
}

