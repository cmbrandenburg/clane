// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// vim: set noet:

#include "check/clane_check.hxx"
#include "../clane_http_message.hxx"

using namespace clane;

int main() {

	http::header_map alpha({{"Content-Length", "0"}});
	http::header_map bravo({{"content-length", "0"}});
	http::header_map charlie({{"content-length", "0"}, {"content-type", "text/plain"}});
	http::header_map delta({{"transfer-encoding", "chunked"}});

	check(alpha == alpha);
	check(!(alpha != alpha));
	check(!(alpha < alpha));
	check(alpha <= alpha);
	check(!(alpha > alpha));
	check(alpha >= alpha);

	check(alpha == bravo);
	check(!(alpha != bravo));
	check(!(alpha < bravo));
	check(alpha <= bravo);
	check(!(alpha > bravo));
	check(alpha >= bravo);

	check(bravo == alpha);
	check(!(bravo != alpha));
	check(!(bravo < alpha));
	check(bravo <= alpha);
	check(!(bravo > alpha));
	check(bravo >= alpha);

	check(!(alpha == charlie));
	check(alpha != charlie);
	check(alpha < charlie);
	check(alpha <= charlie);
	check(!(alpha > charlie));
	check(!(alpha >= charlie));

	check(!(alpha == delta));
	check(alpha != delta);
	check(alpha < delta);
	check(alpha <= delta);
	check(!(alpha > delta));
	check(!(alpha >= delta));

	check(!(charlie == delta));
	check(charlie != delta);
	check(charlie < delta);
	check(charlie <= delta);
	check(!(charlie > delta));
	check(!(charlie >= delta));
}

