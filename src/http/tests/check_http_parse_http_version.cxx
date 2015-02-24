// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// vim: set noet:

#include "check/clane_check.hxx"
#include "../clane_http_message.hxx"
#include <cstring>

using clane::http::protocol_version;

void ok(char const *in, protocol_version const &exp_ver) {
	protocol_version ver;
	check(clane::http::parse_http_version(in, in+std::strlen(in), &ver));
	check(ver == exp_ver);
}

void nok(char const *in) {
	protocol_version ver;
	check(!clane::http::parse_http_version(in, in+std::strlen(in), &ver));
}

int main() {
	check_call(&ok, "HTTP/1.0", protocol_version{1, 0});
	check_call(&ok, "HTTP/1.1", protocol_version{1, 1});
	check_call(&ok, "HTTP/1234.5678", protocol_version{1234, 5678});
	check_call(&nok, "");
	check_call(&nok, "HTTP/");
	check_call(&nok, "HTTP/1");
	check_call(&nok, "HTTP/1.");
	check_call(&nok, "HTTP/12.");
	check_call(&nok, " HTTP/1.1");
	check_call(&nok, " HTTP/1.1 ");
	check_call(&nok, "HTTP1.1");
}

