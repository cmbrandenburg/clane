// vim: set noet:

#include "../http.cpp"
#include "../../check/check.h"

int main() {

	check(clane::http::is_method_valid("alphabravo"));
	check(!clane::http::is_method_valid(""));
	{
		std::string s("alpha");
		s.push_back(0);
		s.append("bravo");
		check(!clane::http::is_method_valid(s));
	}
	check(!clane::http::is_method_valid("alpha\x01" "bravo"));
	check(!clane::http::is_method_valid("alpha\x02" "bravo"));
	check(!clane::http::is_method_valid("alpha\x03" "bravo"));
	check(!clane::http::is_method_valid("alpha\x04" "bravo"));
	check(!clane::http::is_method_valid("alpha\x05" "bravo"));
	check(!clane::http::is_method_valid("alpha\x06" "bravo"));
	check(!clane::http::is_method_valid("alpha\x07" "bravo"));
	check(!clane::http::is_method_valid("alpha\x08" "bravo"));
	check(!clane::http::is_method_valid("alpha\x09" "bravo"));
	check(!clane::http::is_method_valid("alpha\x0a" "bravo"));
	check(!clane::http::is_method_valid("alpha\x0b" "bravo"));
	check(!clane::http::is_method_valid("alpha\x0c" "bravo"));
	check(!clane::http::is_method_valid("alpha\x0d" "bravo"));
	check(!clane::http::is_method_valid("alpha\x0e" "bravo"));
	check(!clane::http::is_method_valid("alpha\x0f" "bravo"));
	check(!clane::http::is_method_valid("alpha\x10" "bravo"));
	check(!clane::http::is_method_valid("alpha\x11" "bravo"));
	check(!clane::http::is_method_valid("alpha\x12" "bravo"));
	check(!clane::http::is_method_valid("alpha\x13" "bravo"));
	check(!clane::http::is_method_valid("alpha\x14" "bravo"));
	check(!clane::http::is_method_valid("alpha\x15" "bravo"));
	check(!clane::http::is_method_valid("alpha\x16" "bravo"));
	check(!clane::http::is_method_valid("alpha\x17" "bravo"));
	check(!clane::http::is_method_valid("alpha\x18" "bravo"));
	check(!clane::http::is_method_valid("alpha\x19" "bravo"));
	check(!clane::http::is_method_valid("alpha\x1a" "bravo"));
	check(!clane::http::is_method_valid("alpha\x1b" "bravo"));
	check(!clane::http::is_method_valid("alpha\x1c" "bravo"));
	check(!clane::http::is_method_valid("alpha\x1d" "bravo"));
	check(!clane::http::is_method_valid("alpha\x1e" "bravo"));
	check(!clane::http::is_method_valid("alpha\x1f" "bravo"));
	check(!clane::http::is_method_valid("alpha(bravo"));
	check(!clane::http::is_method_valid("alpha)bravo"));
	check(!clane::http::is_method_valid("alpha<bravo"));
	check(!clane::http::is_method_valid("alpha>bravo"));
	check(!clane::http::is_method_valid("alpha@bravo"));
	check(!clane::http::is_method_valid("alpha,bravo"));
	check(!clane::http::is_method_valid("alpha;bravo"));
	check(!clane::http::is_method_valid("alpha:bravo"));
	check(!clane::http::is_method_valid("alpha\\bravo"));
	check(!clane::http::is_method_valid("alpha\"bravo"));
	check(!clane::http::is_method_valid("alpha/bravo"));
	check(!clane::http::is_method_valid("alpha[bravo"));
	check(!clane::http::is_method_valid("alpha]bravo"));
	check(!clane::http::is_method_valid("alpha?bravo"));
	check(!clane::http::is_method_valid("alpha=bravo"));
	check(!clane::http::is_method_valid("alpha{bravo"));
	check(!clane::http::is_method_valid("alpha}bravo"));
	check(!clane::http::is_method_valid("alpha bravo"));
	check(!clane::http::is_method_valid("alpha\tbravo"));
	check(!clane::http::is_method_valid("alpha\x7f" "bravo"));

	return 0;
}

