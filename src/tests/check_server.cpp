#include "../clane_http.hpp"
#include <iostream>

void handler(clane::http::server_transaction &xact) {
	xact << "Hello, world.\n";
}

int main(int argc, char **argv) {
	boost::asio::io_service ios{};
	auto srv = clane::http::make_server(ios, &handler);
	srv.add_listener(1==argc ? "localhost:8080" : argv[1]);
	ios.run();
}

