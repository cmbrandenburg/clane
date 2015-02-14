#include "clane/clane_http.hpp"
#include <iostream>

void handler(clane::http::server_transaction &xact) {
	xact << "Hello, world.\n";
}

int main(int argc, char **argv) {
	return 0;
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
}
