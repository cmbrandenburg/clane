// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// vim: set noet:

#include "clane_http_check_parser.hxx"
#include "../clane_http_message.hxx"
#include <cstring>

void ok(clane::http::v1x_request_line_parser &parser, char const *good, char const *exp_method, char const *exp_uri,
		unsigned exp_major_ver, unsigned exp_minor_ver)
{
	make_ok_checker(parser, good, [&parser, exp_method, exp_uri, exp_major_ver, exp_minor_ver]() {
		check(parser.method() == exp_method);
		check(parser.uri().string() == exp_uri);
		check(parser.major_version() == exp_major_ver);
		check(parser.minor_version() == exp_minor_ver);
	})();
}

void nok(clane::http::v1x_request_line_parser &parser, char const *good, char const *bad) {
	make_nok_checker(parser, good, bad, [&parser]() {
		check(parser.status_code() == clane::http::status_code::bad_request);
	})();
}

int main() {
	clane::http::v1x_request_line_parser p;

	// Case: empty line
	check_call(&nok, p, "\r", "\n");

	// Case: valid request line
	check_call(&ok, p, "GET / HTTP/1.1\r\n", "GET", "/", 1, 1);

	// Case: valid request line without carriage return
	check_call(&ok, p, "GET / HTTP/1.1\n", "GET", "/", 1, 1);

	// Case: invalid method
	check_call(&nok, p, "{} / HTTP/1.1\r", "\n");

	// Case: invalid URI
	check_call(&nok, p, "GET :blah HTTP/1.1\r", "\n");

	// Case: invalid HTTP version
	check_call(&nok, p, "GET / HTTP/1.\r", "\n");

	// Case: '*' instead of URI
	check_call(&ok, p, "OPTIONS * HTTP/1.1\r\n", "OPTIONS", "*", 1, 1);

	// Case: non-trivial URI with path normalization
	check_call(&ok, p, "POST http://example.com/alpha/bravo/../?charlie=delta#echo HTTP/1.0\r\n",
			"POST", "http://example.com/alpha/?charlie=delta#echo", 1, 0);
}

