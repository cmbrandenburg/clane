// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// vim: set noet:

#include "check/clane_check.hxx"
#include "../clane_uri.hxx"

int main() {
	auto u = clane::uri::parse_uri_reference("http://alpha/bravo/../charlie//delta");
	u.normalize_path();
	check("http://alpha/charlie/delta" == u.string());
}

