// vim: set noet:

#include "../../check/check.h"
#include "../http_consume.hpp"
#include <cstring>

using namespace clane;

void check_ok(char const *content, char const *exp_method, char const *exp_uri, int exp_major, int exp_minor,
	   	http::header_map const &exp_hdrs) {

	std::string const s = std::string(content) + "extra";
	http::request got_req;

	// single pass:
	http::request_1x_consumer cons(got_req);
	check(cons.consume(s.data(), s.size()));
	check(strlen(content) == cons.total_length());
	check(got_req.method == exp_method);
	check(got_req.uri.to_string() == exp_uri);
	check(got_req.major_version == exp_major);
	check(got_req.minor_version == exp_minor);
	check(got_req.headers == exp_hdrs);

	// byte-by-byte:
	got_req = http::request{};
	cons.reset(got_req);
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
	check(got_req.method == exp_method);
	check(got_req.uri.to_string() == exp_uri);
	check(got_req.major_version == exp_major);
	check(got_req.minor_version == exp_minor);
	check(got_req.headers == exp_hdrs);
}

void check_nok(size_t len_limit, char const *s, http::status_code exp_error_code) {
	http::request got_req;
	// single pass:
	http::request_1x_consumer cons(got_req);
	cons.set_length_limit(len_limit);
	check(cons.consume(s, strlen(s)));
	check(!cons);
	check(exp_error_code == cons.error_code());
}

int main() {

	check_ok("GET / HTTP/1.1\r\n\r\n", "GET", "/", 1, 1, http::header_map{});
	check_ok("GET / HTTP/1.1\r\nalpha: bravo\r\n\r\n", "GET", "/", 1, 1, http::header_map{
			http::header_map::value_type("alpha", "bravo")});

	// not-OK: request line error
	check_nok(0, "GET / \r\nalpha: bravo\r\n\r\n", http::status_code::bad_request);

	// not-OK: headers error
	check_nok(0, "GET / HTTP/1.1\r\nalp\tha: bravo\r\n\r\n", http::status_code::bad_request);

	// not-OK: request line length limit
	check_nok(5, "GET / HTTP/1.1\r\nalpha: bravo\r\n\r\n", http::status_code::request_uri_too_long);

	// not-OK: headers length limit
	check_nok(20, "GET / HTTP/1.1\r\nalpha: bravo\r\n\r\n", http::status_code::bad_request);

	return 0;
}

