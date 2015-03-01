// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// vim: set noet:

#include "clane_http_check_parser.hxx"
#include "http/clane_http_message.hxx"
#include <cstring>

void ok(clane::http::v1x_headers_parser &parser, char const *good, clane::http::header_map const &exp_hdrs) {
	clane::http::header_map got_hdrs;
	make_ok_checker(parser, good, [&parser, &got_hdrs]() {
			parser.reset(&got_hdrs);
		}, [&parser, &got_hdrs, &exp_hdrs]() {
			check(got_hdrs == exp_hdrs);
		})();
}

void nok(clane::http::v1x_headers_parser &parser, char const *good, char const *bad) {
	clane::http::header_map got_hdrs;
	make_nok_checker(parser, good, bad, [&parser, &got_hdrs]() {
		   parser.reset(&got_hdrs);
		}, [&parser]() {
			check(parser.status_code() == clane::http::status_code::bad_request);
		})();
}

int main() {
	using clane::http::header;
	using clane::http::header_map;
	clane::http::v1x_headers_parser p{nullptr};

	// Case: no headers
	check_call(&ok, p, "\r\n", header_map{}); 

	// Case: one header
	check_call(&ok, p, "Content-Length: 1234\r\n\r\n", header_map{header{"content-length", "1234"}});

	// Case: multiple headers
	check_call(&ok, p,
			"Content-Length: 1234\r\n"
			"Host: example.com\r\n"
			"\r\n",
			header_map{
				header{"content-length", "1234"},
				header{"host", "example.com"},
			});

	// Case: no carriage return
	check_call(&ok, p,
			"Content-Length: 1234\n"
			"Host: example.com\n"
			"\n",
			header_map{
				header{"content-length", "1234"},
				header{"host", "example.com"},
			});

	// Case: simple line continuation
	check_call(&ok, p,
			"Cache-Control: no-cache\r\n"
			" no-store\r\n"
			" no-transform\r\n"
			"\r\n",
			header_map{header{"cache-control", "no-cache no-store no-transform"}});

	// Case: line continuation with leading space
	check_call(&ok, p,
			"Cache-Control: no-cache\r\n"
			"  \t no-store\r\n"
			" \t\t\tno-transform\r\n"
			"\r\n",
			header_map{header{"cache-control", "no-cache no-store no-transform"}});

	// Case: line continuation with trailing space
	check_call(&ok, p,
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
		check_call(&nok, p, s.c_str(), "X\r\n");
	}

	// Case: headers are too long (line ends at capacity)
	{
		std::string s{"Content-Length: 1234\r\n"};
		s.append("User-Agent: ");
		while (s.size() < p.capacity()-2)
			s.push_back('X');
		s.append("\r");
		check_call(&nok, p, s.c_str(), "\n\r\n");
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
		check_call(&ok, p, s.c_str(), header_map{
			header{"Content-Length", "1234"},
			header{"User-Agent", val},
		});
	}

	// Case: first line is a line continuation
	check_call(&nok, p, " Cache-Control: no-cache\r", "\n\r\n");
	check_call(&nok, p, "\tCache-Control: no-cache\r", "\n\r\n");

	// Case: invalid header name
	check_call(&nok, p, "[]: alpha\r", "\n\r\n");

	// Case: invalid header value (last header)
	check_call(&nok, p, "alpha: \x01\x02\r\n\r", "\n");

	// Case: invalid header value (not last header)
	check_call(&nok, p, "alpha: \x01\x02\r\ncharlie: delta\r", "\n\r\n");

	// Case: missing ':' separator
	check_call(&nok, p, "alpha bravo\r", "\n\r\n");
}
