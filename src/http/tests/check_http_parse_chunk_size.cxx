// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// vim: set noet:

#include "check/clane_check.hxx"
#include "../clane_http_message.hxx"
#include <cstring>

void ok(char const *in, std::size_t exp_chunk_size) {
	std::size_t chunk_size;
	check(clane::http::parse_chunk_size(in, in+std::strlen(in), &chunk_size));
	check(exp_chunk_size == chunk_size);
}

void nok(char const *in) {
	std::size_t chunk_size = 0;
	check(!clane::http::parse_chunk_size(in, in+std::strlen(in), &chunk_size));
	check(0 == chunk_size);
}

int main() {
	check_call(&ok, "0", 0);
	check_call(&ok, "0", 0);
	check_call(&ok, "1", 1);
	check_call(&ok, "ff", 255);
	check_call(&ok, "FF", 255);
	check_call(&ok, "1234ABcd", 0x01234abcd);
	check_call(&nok, "");
	check_call(&nok, "1g");
	{
		std::string q;
		for (std::size_t i = 0; i < sizeof(std::size_t)*2+1; ++i)
			q.push_back('F');
		check_call(&nok, q.c_str());
	}
}

