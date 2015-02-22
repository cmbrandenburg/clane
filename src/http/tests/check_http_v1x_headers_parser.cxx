// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// vim: set noet:

#include "check/clane_check.hxx"
#include "../clane_http_message.hxx"
#include "clane_http_parser_check.hxx"
#include <cstring>

void okay(clane::http::v1x_headers_parser &p, char const *in, clane::http::header_map const &exp_hdrs) {

	// parse the entire input string all at once:
	p.reset();
	std::string q{in};
	q.append("EXTRA");
	auto n = p.parse(q.data(), q.size());
	check(p.okay());
	check(p.done());
	check(n == std::strlen(in));
	check(p.headers() == exp_hdrs);

	// parse the input one character at a time:
	p.reset();
	for (std::size_t i = 0; i+1 < std::strlen(in); ++i) {
		n = p.parse(in+i, 1);
		check(p.okay());
		check(!p.done());
		check(1 == n);
	}
	n = p.parse(in+std::strlen(in)-1, 1);
	check(p.okay());
	check(p.done());
	check(1 == n);
	check(p.headers() == exp_hdrs);
}

void not_okay(clane::http::v1x_headers_parser &p, char const *ok, char const *nok, clane::http::status_code exp_stat_code) {

	// parse the entire input string all at once:
	p.reset();
	std::string q{ok};
	q.append(nok);
	auto n = p.parse(q.data(), q.size());
	check(!p.okay());
	check(0 == n);
	check(p.status_code() == exp_stat_code);

	// parse the input one character at a time:
	p.reset();
	for (std::size_t i = 0; i < std::strlen(ok); ++i) {
		n = p.parse(ok+i, 1);
		check(p.okay());
		check(!p.done());
		check(1 == n);
	}
	n = p.parse(nok, 1);
	check(!p.okay());
	check(0 == n);
	check(p.status_code() == exp_stat_code);
}

int main() {
	using clane::http::header;
	using clane::http::header_map;
	clane::http::v1x_headers_parser p;

	// Case: no headers
	check_call(&okay, p, "\r\n", header_map{});

	// Case: one header
	check_call(&okay, p, "Content-Length: 1234\r\n\r\n", header_map{header{"content-length", "1234"}});

	// Case: multiple headers
	check_call(&okay, p,
			"Content-Length: 1234\r\n"
			"Host: example.com\r\n"
			"\r\n",
			header_map{
				header{"content-length", "1234"},
				header{"host", "example.com"},
			});

	// Case: no carriage return
	check_call(&okay, p,
			"Content-Length: 1234\n"
			"Host: example.com\n"
			"\n",
			header_map{
				header{"content-length", "1234"},
				header{"host", "example.com"},
			});

	// Case: simple line continuation
	check_call(&okay, p,
			"Cache-Control: no-cache\r\n"
			" no-store\r\n"
			" no-transform\r\n"
			"\r\n",
			header_map{header{"cache-control", "no-cache no-store no-transform"}});

	// Case: line continuation with leading space
	check_call(&okay, p,
			"Cache-Control: no-cache\r\n"
			"  \t no-store\r\n"
			" \t\t\tno-transform\r\n"
			"\r\n",
			header_map{header{"cache-control", "no-cache no-store no-transform"}});

	// Case: line continuation with trailing space
	check_call(&okay, p,
			"Cache-Control: no-cache \t \r\n"
			" no-store    \r\n"
			" no-transform\t\t\t\r\n"
			"\r\n",
			header_map{header{"cache-control", "no-cache no-store no-transform"}});
}

