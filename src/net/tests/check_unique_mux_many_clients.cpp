// vim: set noet:

#include "inc_server.h"
#include "../net_mux.h"
#include "../../check/check.h"
#include <future>
#include <list>

int main() {

	// run multiplexer:
	clane::net::unique_mux mux;
	auto listener = std::make_shared<clane::inc_server_listener>(clane::net::listen_tcp("localhost:", 16));
	mux.add_signal(listener);
	auto fut = std::async(std::launch::async, &clane::net::unique_mux::run, &mux);

	// run clients:
	std::list<std::future<void>> client_futs;
	for (int i = 0; i < 10; ++i) {
		client_futs.push_back(std::async(std::launch::async, clane::inc_client, i + 1, listener->local_addr(), 1000));	
	}

	// cleanup:
	while (!client_futs.empty()) {
		client_futs.front().wait();
		client_futs.pop_front();
	}
	mux.cancel();
	fut.wait();
	return 0;
}

