// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// vim: set noet:

#include "clane_http_check_parser.hxx"
#include "http/clane_http_message.hxx"
#include <cstring>

void ok(clane::http::v1x_empty_line_parser &parser, char const *good, std::size_t exp_chunk_size) {
	make_ok_checker(parser, good, [&parser]() { parser.reset(); }, []() {});
}

void nok(clane::http::v1x_empty_line_parser &parser, char const *good, char const *bad) {
	make_nok_checker(parser, good, bad, [&parser]() { parser.reset(); }, [&parser]() {
		check(parser.status_code() == clane::http::status_code::bad_request);
	})();
}

int main() {
	clane::http::v1x_empty_line_parser p;
	check_call(&ok, p, "\r\n", 0);
	check_call(&ok, p, "\n", 0);
	check_call(&nok, p, "0", "\r\n");
	check_call(&nok, p, "0", "\n");
}

