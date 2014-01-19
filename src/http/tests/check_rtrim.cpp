// vim: set noet:

#include "../../check/check.h"
#include "../http_consume.hpp"

using namespace clane;

int main() {

	std::string s;
	http::rtrim(s);
	check("" == s);

	s = "alpha bravo";
	http::rtrim(s);
	check("alpha bravo" == s);

	s = "alpha bravo \t\r\n";
	http::rtrim(s);
	check("alpha bravo" == s);
}

