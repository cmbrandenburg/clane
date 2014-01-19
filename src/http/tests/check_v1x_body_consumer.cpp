// vim: set noet:

#include "../../check/check.h"
#include "../http_consume.hpp"
#include <cstring>

using namespace clane;

void check_ok(http::v1x_body_consumer::length_type len_type, size_t len, char const *content, std::string const &exp) {

	std::string got;

	// single pass--or as few passes as possible, given that the consumer returns
	// incomplete when the body is chunked:
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

int main() {

	// fixed:
	check_ok(http::v1x_body_consumer::fixed, 0, "", "");
	check_ok(http::v1x_body_consumer::fixed, 13, "Hello, world.", "Hello, world.");

	// infinite:
	check_ok(http::v1x_body_consumer::infinite, 0, "Hello, world.", "Hello, world.");

	// chunked, no error:
	check_ok(http::v1x_body_consumer::chunked, 0, "d\r\nHello, world.\r\n0\r\n", "Hello, world.");
	check_ok(http::v1x_body_consumer::chunked, 0, "7\r\nHello, \r\n6\r\nworld.\r\n0\r\n", "Hello, world.");

	// chunked, error:
	return 99; // FIXME: implement
}

