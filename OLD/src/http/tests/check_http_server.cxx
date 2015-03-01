// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// vim: set noet:

#include "http/clane_http_server.hxx"
#include <iostream>

#if 0
void handler(clane::http::server_transaction &xact) {
	xact << "Hello, world.\n";
}
#endif // #if 0

int main(int argc, char **argv) {
	return 0;
#if 0 // FIXME
	boost::asio::io_service ios{};
	auto srv = clane::http::make_server(ios, &handler);
	srv.add_listener(1==argc ? "localhost:8080" : argv[1]);

	boost::asio::signal_set sigs{ios};
	sigs.add(SIGINT);
	sigs.async_wait([&srv](boost::system::error_code const &ec, int signo) {
		srv.close();
	});

	ios.run();

	// FIXME: Make this test do something useful. For now, it tests that the
	// templates compile okay.
#endif // #if 0
}
