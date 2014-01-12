// vim: set noet:

#include "../../check/check.h"
#include "../uri.cpp"
#include <cctype>

int main() {

	auto char_test = [](char c) -> bool {
		return std::isalpha(c);
	};

	std::ostringstream ss;

	// empty string:
	ss.str("");
	clane::uri::percent_encode(ss, "", char_test);
	check("" == ss.str());

	// valid characters:
	ss.str("");
	clane::uri::percent_encode(ss, "abcdef", char_test);
	check("abcdef" == ss.str());

	// invalid characters:
	ss.str("");
	clane::uri::percent_encode(ss, "0123456789:;<=>?", char_test);
	check("%30%31%32%33%34%35%36%37%38%39%3A%3B%3C%3D%3E%3F" == ss.str());

	return 0;
}


