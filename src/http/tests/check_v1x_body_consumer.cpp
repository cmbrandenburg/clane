// vim: set noet:

#include "../../check/check.h"
#include "../http_consume.hpp"
#include <cstring>

using namespace clane;

void check_ok(http::v1x_body_consumer::length_type len_type, size_t len, char const *content, std::string const &exp) {

	std::string got;

	// as few passes as possible, given that the consumer returns incomplete when
	// the body is chunked:
	http::v1x_body_consumer cons(len_type, len);
	size_t offset = 0;
	do {
		size_t stat = cons.consume(content+offset, strlen(content)-offset, offset);
		check(cons.error != stat);
		offset += stat;
		got.append(content+cons.offset(), cons.size());
		if (len_type != http::v1x_body_consumer::chunked) {
			check(stat == strlen(content));
			break;
		}
	} while (!cons.done());
	check(len_type == http::v1x_body_consumer::infinite || cons.done());
	check(got == exp);

	// byte-by-byte:
	cons.reset(len_type, len);
	got.clear();
	if (strlen(content)) {
		for (size_t i = 0; i < strlen(content)-1; ++i) {
			check(0 == cons.consume("", 0));
			check(!cons.done());
			got.append(content+cons.offset(), cons.size());
			check(1 == cons.consume(content+i, 1));
			check(!cons.done());
			got.append(content+cons.offset(), cons.size());
		}
		check(0 == cons.consume("", 0));
		check(!cons.done());
	}
	got.append(content+cons.offset(), cons.size());
	check(1 == cons.consume(content+strlen(content)-1, 1));
	check(len_type == http::v1x_body_consumer::infinite || cons.done());
	got.append(content+cons.offset(), cons.size());
}

void check_nok(char const *ok, char const *bad, http::status_code exp_error_code) {

	std::string full(ok);
	full.append(bad);

	// as few passes as possible:
	http::v1x_body_consumer cons(http::v1x_body_consumer::chunked, 0);
	size_t offset = 0;
	while (true) {
		size_t stat = cons.consume(full.data()+offset, full.size()-offset);
		if (cons.error == stat)
			break;
		offset += stat;
	}
	check(cons.done());
	check(exp_error_code == cons.error_code());

	// byte-by-byte:
	cons.reset(http::v1x_body_consumer::chunked, 0);
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
	check(exp_error_code == cons.error_code());
}

int main() {

	// fixed:
	check_ok(http::v1x_body_consumer::fixed, 0, "", "");
	check_ok(http::v1x_body_consumer::fixed, 13, "Hello, world.", "Hello, world.");

	// infinite:
	check_ok(http::v1x_body_consumer::infinite, 0, "Hello, world.", "Hello, world.");

	// one chunk, no error:
	check_ok(http::v1x_body_consumer::chunked, 0, "d\r\nHello, world.\r\n0\r\n", "Hello, world.");
	check_ok(http::v1x_body_consumer::chunked, 0, "d\nHello, world.\r\n0\r\n", "Hello, world.");
	check_ok(http::v1x_body_consumer::chunked, 0, "d\r\nHello, world.\n0\r\n", "Hello, world.");
	check_ok(http::v1x_body_consumer::chunked, 0, "d\r\nHello, world.\r\n0\n", "Hello, world.");
	check_ok(http::v1x_body_consumer::chunked, 0, "d\nHello, world.\n0\n", "Hello, world.");

	// two chunks, no error:
	check_ok(http::v1x_body_consumer::chunked, 0, "7\r\nHello, \r\n6\r\nworld.\r\n0\r\n", "Hello, world.");
	check_ok(http::v1x_body_consumer::chunked, 0, "7\nHello, \r\n6\r\nworld.\r\n0\r\n", "Hello, world.");
	check_ok(http::v1x_body_consumer::chunked, 0, "7\r\nHello, \n6\r\nworld.\r\n0\r\n", "Hello, world.");
	check_ok(http::v1x_body_consumer::chunked, 0, "7\r\nHello, \r\n6\nworld.\r\n0\r\n", "Hello, world.");
	check_ok(http::v1x_body_consumer::chunked, 0, "7\r\nHello, \r\n6\r\nworld.\n0\r\n", "Hello, world.");
	check_ok(http::v1x_body_consumer::chunked, 0, "7\r\nHello, \r\n6\r\nworld.\r\n0\n", "Hello, world.");

	// multi-digit chunk length:
	check_ok(http::v1x_body_consumer::chunked, 0, "1a\r\nHello, world. How are you?\r\n0\r\n", "Hello, world. How are you?");

	// chunked, error:
	check_nok("", "\r\nHello, world.\r\n0\r\n", http::status_code::bad_request); // missing first chunk size
	check_nok("d\r\nHello, world.", "0\r\n0\r\n", http::status_code::bad_request); // expected \r or \n, got 0
	check_nok("d\r\nHello, world.\r", "\r\n0\r\n", http::status_code::bad_request); // expected \n, got \r
	check_nok("d\r\nHello, world.\r\n0", "invalid\r\n", http::status_code::bad_request); // expected \r or \n, got i
	check_nok("d\r\nHello, world.\r\n0\r", "invalid\n", http::status_code::bad_request); // expected \r or \n, got i

}

