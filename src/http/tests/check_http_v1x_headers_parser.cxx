// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// vim: set noet:

#include "check/clane_check.hxx"
#include "../clane_http_message.hxx"
#include <cstring>

void okay(clane::http::v1x_headers_parser &p, char const *in, clane::http::header_map const &exp_hdrs) {

	// parse the entire input string all at once:
	p.reset();
	std::string q{in};
	q.append("EXTRA");
	auto n = p.parse(q.data(), q.size());
	check(!p.bad());
	check(p.fin());
	check(n == std::strlen(in));
	check(p.headers() == exp_hdrs);

	// parse the input one character at a time:
	p.reset();
	for (std::size_t i = 0; i+1 < std::strlen(in); ++i) {
		n = p.parse(in+i, 1);
		check(!p.bad());
		check(!p.fin());
		check(1 == n);
	}
	n = p.parse(in+std::strlen(in)-1, 1);
	check(!p.bad());
	check(p.fin());
	check(1 == n);
	check(p.headers() == exp_hdrs);
}

void not_okay(clane::http::v1x_headers_parser &p, char const *ok, char const *nok, clane::http::status_code exp_stat_code) {

	// parse the entire input string all at once:
	p.reset();
	std::string q{ok};
	q.append(nok);
	auto n = p.parse(q.data(), q.size());
	check(p.bad());
	check(0 == n);
	check(p.status_code() == exp_stat_code);

	// parse the input one character at a time:
	p.reset();
	for (std::size_t i = 0; i < std::strlen(ok); ++i) {
		n = p.parse(ok+i, 1);
		check(!p.bad());
		check(!p.fin());
		check(1 == n);
	}
	n = p.parse(nok, 1);
	check(p.bad());
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

	// Case: headers are too long (line extends too long)
	{
		std::string s{"Content-Length: 1234\r\n"};
		s.append("User-Agent: ");
		while (s.size() < p.capacity()-1)
			s.push_back('X');
		check_call(&not_okay, p, s.c_str(), "X\r\n", clane::http::status_code::bad_request);
	}

	// Case: headers are too long (line ends at capacity)
	{
		std::string s{"Content-Length: 1234\r\n"};
		s.append("User-Agent: ");
		while (s.size() < p.capacity()-2)
			s.push_back('X');
		s.append("\r\n");
		check_call(&not_okay, p, s.c_str(), "\r\n", clane::http::status_code::bad_request);
	}

	// Case: headers are exactly as long as possible
	{
		std::string s{"Content-Length: 1234\r\n"};
		s.append("User-Agent: ");
		std::string val;
		while (s.size() < p.capacity()-4) {
			s.push_back('X');
			val.push_back('X');
		}
		s.append("\r\n\r\n");
		check_call(&okay, p, s.c_str(), header_map{
			header{"Content-Length", "1234"},
			header{"User-Agent", val},
		});
	}

	// Case: first line is a line continuation
	check_call(&not_okay, p,
			" Cache-Control: no-cache\r",
			"\n\r\n",
			clane::http::status_code::bad_request);
	check_call(&not_okay, p,
			"\tCache-Control: no-cache\r",
			"\n\r\n",
			clane::http::status_code::bad_request);

	// Case: invalid header name
	check_call(&not_okay, p, "[]: alpha\r", "\n\r\n", clane::http::status_code::bad_request);

	// Case: invalid header value (last header)
	check_call(&not_okay, p, "alpha: \x01\x02\r\n\r", "\n", clane::http::status_code::bad_request);

	// Case: invalid header value (not last header)
	check_call(&not_okay, p, "alpha: \x01\x02\r\ncharlie: delta\r", "\n\r\n", clane::http::status_code::bad_request);

	// Case: missing ':' separator
	check_call(&not_okay, p, "alpha bravo\r", "\n\r\n", clane::http::status_code::bad_request);
}

