#include "check/check.h"
#include "../clane_http_parse.hpp"

int main() {
	using clane::http::parse_line;

	bool complete;
	std::string line;

	// Case: empty string
	line = "alpha ";
	check(0 == parse_line("", 0, 100, line, complete));
	check(!complete);
	check(line == "alpha ");

	// Case: incomplete line
	line = "alpha ";
	check(13 == parse_line("bravo charlie", 13, 100, line, complete));
	check(!complete);
	check(line == "alpha bravo charlie");

	// Case: complete line ("\n")
	line = "alpha ";
	check(14 == parse_line("bravo charlie\ndelta", 19, 100, line, complete));
	check(complete);
	check(line == "alpha bravo charlie");

	// Case: complete line ("\r\n")
	line = "alpha ";
	check(15 == parse_line("bravo charlie\r\ndelta", 20, 100, line, complete));
	check(complete);
	check(line == "alpha bravo charlie");

	// Case: Input buffer starts with "\n"
	line = "alpha";
	check(1 == parse_line("\nbravo", 6, 100, line, complete));
	check(complete);
	check(line == "alpha");

	// Case: Input buffer starts with "\n", preexisting line ends with "\r"
	line = "alpha\r";
	check(1 == parse_line("\nbravo", 6, 100, line, complete));
	check(complete);
	check(line == "alpha");

	// Case: incomplete line, but too long
	line = "alpha ";
	check(0 == parse_line("bravo charlie", 13, 10, line, complete));

	// Case: complete line, but too long
	line = "alpha ";
	check(0 == parse_line("bravo\ncharlie", 13, 10, line, complete));
}

