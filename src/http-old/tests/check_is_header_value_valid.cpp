// vim: set noet:

#include "../http.cpp"
#include "../../check/check.h"

int main() {

	check_true(clane::http::is_header_value_valid(""));
	check_true(clane::http::is_header_value_valid("alphabravo"));
	check_true(clane::http::is_header_value_valid("alpha bravo"));
	check_true(clane::http::is_header_value_valid("alpha\tbravo"));

	// separators:
	check_true(clane::http::is_header_value_valid("alpha(bravo"));
	check_true(clane::http::is_header_value_valid("alpha)bravo"));
	check_true(clane::http::is_header_value_valid("alpha<bravo"));
	check_true(clane::http::is_header_value_valid("alpha>bravo"));
	check_true(clane::http::is_header_value_valid("alpha@bravo"));
	check_true(clane::http::is_header_value_valid("alpha,bravo"));
	check_true(clane::http::is_header_value_valid("alpha;bravo"));
	check_true(clane::http::is_header_value_valid("alpha:bravo"));
	check_true(clane::http::is_header_value_valid("alpha\\bravo"));
	check_true(clane::http::is_header_value_valid("alpha\"bravo"));
	check_true(clane::http::is_header_value_valid("alpha/bravo"));
	check_true(clane::http::is_header_value_valid("alpha[bravo"));
	check_true(clane::http::is_header_value_valid("alpha]bravo"));
	check_true(clane::http::is_header_value_valid("alpha?bravo"));
	check_true(clane::http::is_header_value_valid("alpha=bravo"));
	check_true(clane::http::is_header_value_valid("alpha{bravo"));
	check_true(clane::http::is_header_value_valid("alpha}bravo"));

	// non-ASCII characters
	for (int i = 128; i < 256; ++i) {
		std::string s("alpha");
		s.push_back(static_cast<char>(i));
		s.append("bravo");
		check_true(clane::http::is_header_value_valid(s));
	}

	{
		std::string s("alpha");
		s.push_back(0);
		s.append("bravo");
		check_false(clane::http::is_header_value_valid(s));
	}
	check_false(clane::http::is_header_value_valid("alpha\x01" "bravo"));
	check_false(clane::http::is_header_value_valid("alpha\x02" "bravo"));
	check_false(clane::http::is_header_value_valid("alpha\x03" "bravo"));
	check_false(clane::http::is_header_value_valid("alpha\x04" "bravo"));
	check_false(clane::http::is_header_value_valid("alpha\x05" "bravo"));
	check_false(clane::http::is_header_value_valid("alpha\x06" "bravo"));
	check_false(clane::http::is_header_value_valid("alpha\x07" "bravo"));
	check_false(clane::http::is_header_value_valid("alpha\x08" "bravo"));
	// tab character skipped
	check_false(clane::http::is_header_value_valid("alpha\x0a" "bravo"));
	check_false(clane::http::is_header_value_valid("alpha\x0b" "bravo"));
	check_false(clane::http::is_header_value_valid("alpha\x0c" "bravo"));
	check_false(clane::http::is_header_value_valid("alpha\x0d" "bravo"));
	check_false(clane::http::is_header_value_valid("alpha\x0e" "bravo"));
	check_false(clane::http::is_header_value_valid("alpha\x0f" "bravo"));
	check_false(clane::http::is_header_value_valid("alpha\x10" "bravo"));
	check_false(clane::http::is_header_value_valid("alpha\x11" "bravo"));
	check_false(clane::http::is_header_value_valid("alpha\x12" "bravo"));
	check_false(clane::http::is_header_value_valid("alpha\x13" "bravo"));
	check_false(clane::http::is_header_value_valid("alpha\x14" "bravo"));
	check_false(clane::http::is_header_value_valid("alpha\x15" "bravo"));
	check_false(clane::http::is_header_value_valid("alpha\x16" "bravo"));
	check_false(clane::http::is_header_value_valid("alpha\x17" "bravo"));
	check_false(clane::http::is_header_value_valid("alpha\x18" "bravo"));
	check_false(clane::http::is_header_value_valid("alpha\x19" "bravo"));
	check_false(clane::http::is_header_value_valid("alpha\x1a" "bravo"));
	check_false(clane::http::is_header_value_valid("alpha\x1b" "bravo"));
	check_false(clane::http::is_header_value_valid("alpha\x1c" "bravo"));
	check_false(clane::http::is_header_value_valid("alpha\x1d" "bravo"));
	check_false(clane::http::is_header_value_valid("alpha\x1e" "bravo"));
	check_false(clane::http::is_header_value_valid("alpha\x1f" "bravo"));
	check_false(clane::http::is_header_value_valid("alpha\x7f" "bravo"));

	return 0;
}
