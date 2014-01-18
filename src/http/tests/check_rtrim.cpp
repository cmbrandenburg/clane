// vim: set noet:

#include "../http_consume.cpp"
#include "../../check/check.h"

using namespace clane;

int main() {
	return 77;
#if 0

	std::string s;
	http::rtrim(s);
	check("" == s);

	s = "alpha bravo";
	http::rtrim(s);
	check("alpha bravo" == s);

	s = "alpha bravo \t\r\n";
	http::rtrim(s);
	check("alpha bravo" == s);
#endif
}

