// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// vim: set noet:

#include "clane_http_check_parser.hxx"
#include "../clane_http_message.hxx"
#include <cstring>

void ok(clane::http::v1x_chunk_size_parser &parser, char const *good, std::size_t exp_chunk_size) {
	std::size_t chunk_size;
	make_ok_checker(parser, good, [&parser, &chunk_size]() {
		parser.reset(&chunk_size);
	}, [&chunk_size, exp_chunk_size]() {
		check(chunk_size == exp_chunk_size);
	})();
}

void nok(clane::http::v1x_chunk_size_parser &parser, char const *good, char const *bad) {
	std::size_t chunk_size;
	make_nok_checker(parser, good, bad, [&parser, &chunk_size]() {
		parser.reset(&chunk_size);
	}, [&parser]() {
		check(parser.status_code() == clane::http::status_code::bad_request);
	})();
}

int main() {
	clane::http::v1x_chunk_size_parser p{nullptr};
	check_call(&ok, p, "0\r\n", 0);
	check_call(&ok, p, "0\n", 0);
	check_call(&ok, p, "12Ab\r\n", 0x12ab);
	check_call(&ok, p, "12Ab\n", 0x12ab);
	check_call(&nok, p, "xyz\r", "\n");
	check_call(&nok, p, "\r", "\n");
}

