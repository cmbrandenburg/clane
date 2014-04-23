// vim: set noet:

#include "../clane_http_parse.hpp"
#include "../../clane_check.hpp"

using namespace clane;

#define check_ok(s) \
	do { \
		check(http::is_header_name_valid(s)); \
	} while (false)

#define check_nok(s) \
	do { \
		check(!http::is_header_name_valid(s)); \
	} while (false)

int main() {
	check_ok("alphabravo");
	check_nok("");
	check_nok(std::string("alpha\0bravo", 11));
	check_nok("alpha\x01" "bravo");
	check_nok("alpha\x02" "bravo");
	check_nok("alpha\x03" "bravo");
	check_nok("alpha\x04" "bravo");
	check_nok("alpha\x05" "bravo");
	check_nok("alpha\x06" "bravo");
	check_nok("alpha\x07" "bravo");
	check_nok("alpha\x08" "bravo");
	check_nok("alpha\x09" "bravo");
	check_nok("alpha\x0a" "bravo");
	check_nok("alpha\x0b" "bravo");
	check_nok("alpha\x0c" "bravo");
	check_nok("alpha\x0d" "bravo");
	check_nok("alpha\x0e" "bravo");
	check_nok("alpha\x0f" "bravo");
	check_nok("alpha\x10" "bravo");
	check_nok("alpha\x11" "bravo");
	check_nok("alpha\x12" "bravo");
	check_nok("alpha\x13" "bravo");
	check_nok("alpha\x14" "bravo");
	check_nok("alpha\x15" "bravo");
	check_nok("alpha\x16" "bravo");
	check_nok("alpha\x17" "bravo");
	check_nok("alpha\x18" "bravo");
	check_nok("alpha\x19" "bravo");
	check_nok("alpha\x1a" "bravo");
	check_nok("alpha\x1b" "bravo");
	check_nok("alpha\x1c" "bravo");
	check_nok("alpha\x1d" "bravo");
	check_nok("alpha\x1e" "bravo");
	check_nok("alpha\x1f" "bravo");
	check_nok("alpha(bravo");
	check_nok("alpha)bravo");
	check_nok("alpha<bravo");
	check_nok("alpha>bravo");
	check_nok("alpha@bravo");
	check_nok("alpha,bravo");
	check_nok("alpha;bravo");
	check_nok("alpha:bravo");
	check_nok("alpha\\bravo");
	check_nok("alpha\"bravo");
	check_nok("alpha/bravo");
	check_nok("alpha[bravo");
	check_nok("alpha]bravo");
	check_nok("alpha?bravo");
	check_nok("alpha=bravo");
	check_nok("alpha{bravo");
	check_nok("alpha}bravo");
	check_nok("alpha bravo");
	check_nok("alpha\tbravo");
	check_nok("alpha\x7f" "bravo");
}

