// vim: set noet:

#include "../../check/check.h"
#include "../http_consume.hpp"
#include <cstring>

using namespace clane;

void check_ok(char const *content, int exp_major, int exp_minor, http::status_code exp_stat, char const *exp_reason) {

	std::string const s = std::string(content) + "extra";
	int got_major{};
	int got_minor{};
	http::status_code got_stat{};
	std::string got_reason;

	// single pass:
	http::v1x_status_line_consumer cons(got_major, got_minor, got_stat, got_reason);
	check(strlen(content) == cons.consume(s.data(), s.size()));
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
	check(!cons.done());
	for (size_t i = 0; i < strlen(content)-1; ++i) {
		check(0 == cons.consume("", 0));
		check(!cons.done());
		check(1 == cons.consume(s.data()+i, 1));
		check(!cons.done());
	}
	check(0 == cons.consume("", 0));
	check(!cons.done());
	check(1 == cons.consume(s.data()+strlen(content)-1, 1));
	check(cons.done());
	check(got_major == exp_major);
	check(got_minor == exp_minor);
	check(got_stat == exp_stat);
	check(got_reason == exp_reason);
}

void check_nok(size_t len_limit, char const *ok, char const *bad) {

	std::string full(ok);
 	full.append(bad);
	int got_major{};
	int got_minor{};
	http::status_code got_stat{};
	std::string got_reason;

	// single pass:
	http::v1x_status_line_consumer cons(got_major, got_minor, got_stat, got_reason);
	cons.set_length_limit(len_limit);
	check(cons.error == cons.consume(full.data(), full.size()));
	check(cons.done());

	// byte-by-byte:
	got_major = 0;
	got_minor = 0;
	got_stat = http::status_code{};
	got_reason.clear();
	cons.reset(got_major, got_minor, got_stat, got_reason);
	for (size_t i = 0; i < strlen(ok); ++i) {
		check(0 == cons.consume("", 0));
		check(!cons.done());
		check(1 == cons.consume(ok+i, 1));
		check(!cons.done());
	}
	check(0 == cons.consume("", 0));
	check(!cons.done());
	check(cons.error == cons.consume(bad, 1));
	check(cons.done());
}

int main() {

	check_ok("HTTP/1.1 200 OK\r\n", 1, 1, http::status_code::ok, "OK");
	check_ok("HTTP/1.1 200 OK\n", 1, 1, http::status_code::ok, "OK");
	check_ok("HTTP/1.1 200 \r\n", 1, 1, http::status_code::ok, "");

	// not-OK: empty line
	check_nok(0, "", "\r\n");

	// not-OK: missing reason phrase
	check_nok(0, "HTTP/1.1 200", "\r\n");

	// not-OK: missing status code
	check_nok(0, "HTTP/1.1 ", "\r\n");
	check_nok(0, "HTTP/1.1", "\r\n");

	// not-OK: invalid version
	check_nok(0, "HTTP/1.", " 200 OK\r\n");
}

