// vim: set noet:

#include "../uri.cpp"
#include "../../check/check.h"
#include <cctype>

int main() {

	auto char_test = [](char c) -> bool {
		return std::isalpha(c);
	};

	// empty string:
	std::string s = "";
	check(clane::uri::percent_decode(s, char_test));

	// no percent sign, valid characters:
	s = "abcdef";
	check(clane::uri::percent_decode(s, char_test));
	check("abcdef" == s);

	// no percent sign, invalid characters:
	s = "0123456789";
	check(!clane::uri::percent_decode(s, char_test));

	// valid percent:
	s = "%20alpha";
	check(clane::uri::percent_decode(s, char_test));
	check(" alpha" == s);

	// multiple valid percents:
	s = "%61%6c%70%68%61";
	check(clane::uri::percent_decode(s, char_test));
	check("alpha" == s);

	// upper case hex:
	s = "%61%6C%70%68%61";
	check(clane::uri::percent_decode(s, char_test));
	check("alpha" == s);

	// percents decode to "invalid" chars, still tests okay:
	s = "%31%32%33%34%35";
	check(clane::uri::percent_decode(s, char_test));
	check("12345" == s);

	// error: incomplete percent
	s = "%";
	check(!clane::uri::percent_decode(s, char_test));
	s = "%0";
	check(!clane::uri::percent_decode(s, char_test));
	s = "%0a";
	check(clane::uri::percent_decode(s, char_test));
	check("\n" == s);

	// error: invalid hex digit
	s = "%z12345";
	check(!clane::uri::percent_decode(s, char_test));
	s = "%z12345";
	check(!clane::uri::percent_decode(s, char_test));

	return 0;
}


