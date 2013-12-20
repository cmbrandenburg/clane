// vim: set noet:

#include "../http.cpp"
#include "../../check/check.h"

int main() {

	std::string s;
	clane::http::rtrim(s);
	check_eq("", s);

	s = "alpha bravo";
	clane::http::rtrim(s);
	check_eq("alpha bravo", s);

	s = "alpha bravo \t\r\n";
	clane::http::rtrim(s);
	check_eq("alpha bravo", s);

	return 0;
}

