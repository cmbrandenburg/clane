// vim: set noet:

#include "../http_consumer.h"
#include "../../check/check.h"
#include <cstring>

using namespace clane;

void check_ok(char const *content, size_t exp_size) {

	std::string const s = std::string(content) + "extra";

	// single pass:
	http::chunk_line_consumer cons;
	check(cons.consume(s.data(), s.size()));
	check(strlen(content) == cons.length());
	check(exp_size == cons.chunk_size());

	// byte-by-byte:
	cons.reset();
	for (size_t i = 0; i < strlen(content)-1; ++i) {
		check(!cons.consume("", 0));
		check(cons);
		check(!cons.consume(s.data()+i, 1));
		check(cons);
	}
	check(!cons.consume("", 0));
	check(cons);
	check(cons.consume(s.data()+strlen(content)-1, 1));
	check(strlen(content) == cons.length());
	check(exp_size == cons.chunk_size());
}

void check_nok(size_t len_limit, char const *s, http::status_code exp_error_code) {
	// single pass:
	http::chunk_line_consumer cons;
	cons.set_length_limit(len_limit);
	check(!cons.consume(s, strlen(s)));
	check(!cons);
	check(exp_error_code == cons.error_code());
}

int main() {

	check_ok("0\r\n", 0);
	check_ok("0\n", 0);
	check_ok("123\r\n", 0x123);
	check_ok("123\n", 0x123);
	check_ok("ABC\r\n", 0xabc);
	check_ok("ABC\n", 0xabc);
	check_ok("abc\r\n", 0xabc);
	check_ok("abc\n", 0xabc);
	check_ok("1bC\r\n", 0x1bc);

	return 0;
}

