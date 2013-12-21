// vim: set noet:

#include "../http.cpp"
#include "../../check/check.h"

int main() {
	char const *p;

	p = "";
	char const *got = clane::http::find_newline(p, strlen(p));
	check(p + strlen(p) == got);

	p = "alpha bravo\tcharlie";
	got = clane::http::find_newline(p, strlen(p));
	check(p + strlen(p) == got);

	p = "alpha\nbravo";
	got = clane::http::find_newline(p, strlen(p));
	check(p + 5 == got);

	p = "alpha\nbravo\n";
	got = clane::http::find_newline(p, strlen(p));
	check(p + 5 == got);

	p = "alpha\r\nbravo";
	got = clane::http::find_newline(p, strlen(p));
	check(p + 5 == got);

	p = "alpha\r\nbravo\r\n";
	got = clane::http::find_newline(p, strlen(p));
	check(p + 5 == got);

	p = "alpha\rbravo\r\n";
	got = clane::http::find_newline(p, strlen(p));
	check(p + 11 == got);

	p = "alpha\r";
	got = clane::http::find_newline(p, strlen(p));
	check(p + 5 == got);

	p = "\r\n";
	got = clane::http::find_newline(p, strlen(p));
	check(p == got);

	std::string s("alpha");
	s.push_back(0);
	s.append("bravo\r\n");
	p = s.data();
	got = clane::http::find_newline(p, s.size());
	check(p + 11 == got);

	return 0;
}

