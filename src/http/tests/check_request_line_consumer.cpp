// vim: set noet:

#include "../http_consumer.h"
#include "../../check/check.h"
#include <cstring>

using namespace clane;

void check_ok(char const *content, char const *exp_method, char const *exp_uri, int exp_major, int exp_minor) {

	std::string const s = std::string(content) + "extra";
	std::string got_method;
	uri::uri got_uri;
	int got_major{};
	int got_minor{};

	// single pass:
	http::request_line_consumer cons(got_method, got_uri, got_major, got_minor);
	check(cons.consume(s.data(), s.size()));
	check(strlen(content) == cons.length());
	check(got_method == exp_method);
	check(got_uri.to_string() == exp_uri);
	check(got_major == exp_major);
	check(got_minor == exp_minor);

	// byte-by-byte:
	got_method.clear();
	got_uri.clear();
	got_major = 0;
	got_minor = 0;
	cons.reset(got_method, got_uri, got_major, got_minor);
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
	check(got_method == exp_method);
	check(got_uri.to_string() == exp_uri);
	check(got_major == exp_major);
	check(got_minor == exp_minor);
}

void check_nok(size_t len_limit, char const *s, http::status_code exp_error_code) {
	std::string got_method;
	uri::uri got_uri;
	int got_major{};
	int got_minor{};
	// single pass:
	http::request_line_consumer cons(got_method, got_uri, got_major, got_minor);
	cons.set_length_limit(len_limit);
	check(cons.consume(s, strlen(s)));
	check(!cons);
	check(exp_error_code == cons.error_code());
}

int main() {

	check_ok("GET / HTTP/1.1\r\n", "GET", "/", 1, 1);
	check_ok("GET /foo?bar HTTP/1.1\r\n", "GET", "/foo?bar", 1, 1);
	check_ok("GET /foo?bar HTTP/1.0\r\n", "GET", "/foo?bar", 1, 0);

	// not-OK: empty line
	check_nok(0, "\r\n", http::status_code::bad_request);

	// not-OK: missing version
	check_nok(0, "GET /foo \r\n", http::status_code::bad_request);
	check_nok(0, "GET /foo\r\n", http::status_code::bad_request);

	// not-OK: missing URI
	check_nok(0, "GET \r\n", http::status_code::bad_request);
	check_nok(0, "GET\r\n", http::status_code::bad_request);

	// not-OK: invalid method
	check_nok(0, "GE\tT /foo HTTP/1.1\r\n", http::status_code::bad_request);

	// not-OK: invalid URI
	check_nok(0, "GET /fo\to HTTP/1.1\r\n", http::status_code::bad_request);

	// not-OK: invalid version
	check_nok(0, "GET /foo blah/1.1\r\n", http::status_code::bad_request);
	check_nok(0, "GET /foo http/1.1\r\n", http::status_code::bad_request);
	check_nok(0, "GET /foo HTTP/a.1\r\n", http::status_code::bad_request);
	check_nok(0, "GET /foo HTTP/1.a\r\n", http::status_code::bad_request);
	check_nok(0, "GET /foo HTTP/-1.1\r\n", http::status_code::bad_request);
	check_nok(0, "GET /foo HTTP/1.-1\r\n", http::status_code::bad_request);
	check_nok(0, "GET /foo HTTP/1.1 \r\n", http::status_code::bad_request);

	return 0;
}

