// vim: set noet:

#include "../../check/check.h"
#include "../http_consume.hpp"
#include <cstring>

using namespace clane;

#if 0
void check_ok(char const *content, int exp_major, int exp_minor, http::status_code exp_stat, char const *exp_reason) {

	std::string const s = std::string(content) + "extra";
	int got_major{};
	int got_minor{};
	http::status_code got_stat{};
	std::string got_reason;

	// single pass:
	http::status_line_consumer cons(got_major, got_minor, got_stat, got_reason);
	check(cons.consume(s.data(), s.size()));
	check(strlen(content) == cons.total_length());
	check(got_major == exp_major);
	check(got_minor == exp_minor);
	check(got_stat == exp_stat);
	check(got_reason == exp_reason);

	// byte-by-byte:
	got_major = 0;
	got_minor = 0;
	got_stat = http::status_code{};
	got_reason.clear();
	cons.reset(got_major, got_minor, got_stat, got_reason);
	for (size_t i = 0; i < strlen(content)-1; ++i) {
		check(!cons.consume("", 0));
		check(cons);
		check(!cons.consume(s.data()+i, 1));
		check(cons);
	}
	check(!cons.consume("", 0));
	check(cons);
	check(cons.consume(s.data()+strlen(content)-1, 1));
	check(strlen(content) == cons.total_length());
	check(got_major == exp_major);
	check(got_minor == exp_minor);
	check(got_stat == exp_stat);
	check(got_reason == exp_reason);
}

void check_nok(size_t len_limit, char const *s) {
	int got_major{};
	int got_minor{};
	http::status_code got_stat{};
	std::string got_reason;
	// single pass:
	http::status_line_consumer cons(got_major, got_minor, got_stat, got_reason);
	cons.set_length_limit(len_limit);
	check(cons.consume(s, strlen(s)));
	check(!cons);
}
#endif

int main() {
	return 77;
#if 0

	check_ok("HTTP/1.1 200 OK\r\n", 1, 1, http::status_code::ok, "OK");
	check_ok("HTTP/1.1 200 OK\n", 1, 1, http::status_code::ok, "OK");

	// not-OK: empty line
	check_nok(0, "\r\n");

	// not-OK: missing reason phrase
	check_nok(0, "HTTP/1.1 200\r\n");

	// not-OK: missing status code
	check_nok(0, "HTTP/1.1\r\n");

	// not-OK: invalid version
	check_nok(0, "HTTP/1. 200 OK\r\n");
	check_nok(0, "HTTP/1.-1 200 OK\r\n");
	check_nok(0, "HTTP/1.a 200 OK\r\n");
	check_nok(0, "HTTP/.-1 200 OK\r\n");
	check_nok(0, "HTTP/-1.1 200 OK\r\n");
	check_nok(0, "HTTP/a.1 200 OK\r\n");
	check_nok(0, "HTTP/1.1. 200 OK\r\n");
	check_nok(0, "http/1.1. 200 OK\r\n");
#endif
}

