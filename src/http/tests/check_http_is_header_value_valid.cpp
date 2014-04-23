// vim: set noet:

#include "../clane_http_parse.hpp"
#include "../../clane_check.hpp"

using namespace clane;

int main() {
	check(clane::http::is_header_value_valid(""));
	check(clane::http::is_header_value_valid("alphabravo"));
	check(clane::http::is_header_value_valid("alpha bravo"));
	check(clane::http::is_header_value_valid("alpha\tbravo"));

	// separators:
	check(clane::http::is_header_value_valid("alpha(bravo"));
	check(clane::http::is_header_value_valid("alpha)bravo"));
	check(clane::http::is_header_value_valid("alpha<bravo"));
	check(clane::http::is_header_value_valid("alpha>bravo"));
	check(clane::http::is_header_value_valid("alpha@bravo"));
	check(clane::http::is_header_value_valid("alpha,bravo"));
	check(clane::http::is_header_value_valid("alpha;bravo"));
	check(clane::http::is_header_value_valid("alpha:bravo"));
	check(clane::http::is_header_value_valid("alpha\\bravo"));
	check(clane::http::is_header_value_valid("alpha\"bravo"));
	check(clane::http::is_header_value_valid("alpha/bravo"));
	check(clane::http::is_header_value_valid("alpha[bravo"));
	check(clane::http::is_header_value_valid("alpha]bravo"));
	check(clane::http::is_header_value_valid("alpha?bravo"));
	check(clane::http::is_header_value_valid("alpha=bravo"));
	check(clane::http::is_header_value_valid("alpha{bravo"));
	check(clane::http::is_header_value_valid("alpha}bravo"));

	// non-ASCII characters
	for (int i = 128; i < 256; ++i) {
		std::string s("alpha");
		s.push_back(static_cast<char>(i));
		s.append("bravo");
		check(clane::http::is_header_value_valid(s));
	}

	{
		std::string s("alpha");
		s.push_back(0);
		s.append("bravo");
		check(!clane::http::is_header_value_valid(s));
	}
	check(!clane::http::is_header_value_valid("alpha\x01" "bravo"));
	check(!clane::http::is_header_value_valid("alpha\x02" "bravo"));
	check(!clane::http::is_header_value_valid("alpha\x03" "bravo"));
	check(!clane::http::is_header_value_valid("alpha\x04" "bravo"));
	check(!clane::http::is_header_value_valid("alpha\x05" "bravo"));
	check(!clane::http::is_header_value_valid("alpha\x06" "bravo"));
	check(!clane::http::is_header_value_valid("alpha\x07" "bravo"));
	check(!clane::http::is_header_value_valid("alpha\x08" "bravo"));
	// tab character skipped
	check(!clane::http::is_header_value_valid("alpha\x0a" "bravo"));
	check(!clane::http::is_header_value_valid("alpha\x0b" "bravo"));
	check(!clane::http::is_header_value_valid("alpha\x0c" "bravo"));
	check(!clane::http::is_header_value_valid("alpha\x0d" "bravo"));
	check(!clane::http::is_header_value_valid("alpha\x0e" "bravo"));
	check(!clane::http::is_header_value_valid("alpha\x0f" "bravo"));
	check(!clane::http::is_header_value_valid("alpha\x10" "bravo"));
	check(!clane::http::is_header_value_valid("alpha\x11" "bravo"));
	check(!clane::http::is_header_value_valid("alpha\x12" "bravo"));
	check(!clane::http::is_header_value_valid("alpha\x13" "bravo"));
	check(!clane::http::is_header_value_valid("alpha\x14" "bravo"));
	check(!clane::http::is_header_value_valid("alpha\x15" "bravo"));
	check(!clane::http::is_header_value_valid("alpha\x16" "bravo"));
	check(!clane::http::is_header_value_valid("alpha\x17" "bravo"));
	check(!clane::http::is_header_value_valid("alpha\x18" "bravo"));
	check(!clane::http::is_header_value_valid("alpha\x19" "bravo"));
	check(!clane::http::is_header_value_valid("alpha\x1a" "bravo"));
	check(!clane::http::is_header_value_valid("alpha\x1b" "bravo"));
	check(!clane::http::is_header_value_valid("alpha\x1c" "bravo"));
	check(!clane::http::is_header_value_valid("alpha\x1d" "bravo"));
	check(!clane::http::is_header_value_valid("alpha\x1e" "bravo"));
	check(!clane::http::is_header_value_valid("alpha\x1f" "bravo"));
	check(!clane::http::is_header_value_valid("alpha\x7f" "bravo"));
}

