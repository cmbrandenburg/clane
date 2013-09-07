// vim: set noet:

#include "inc_server.h"
#include "../net_mux.h"
#include "../../check/check.h"
#include <future>
#include <list>

int main() {

	// run multiplexer:
	clane::net::shared_mux mux;
	auto listener = std::make_shared<clane::inc_server_listener>(clane::net::listen_tcp("localhost:", 16));
	mux.add_signal(listener);
	std::list<std::future<void>> futs;
	for (int i = 0; i < 10; ++i)
		futs.push_back(std::async(std::launch::async, &clane::net::shared_mux::run, &mux));

	// run client:
	clane::inc_client(1, listener->local_addr(), 1000);

	// cleanup:
	mux.cancel();
	while (!futs.empty()) {
		futs.front().wait();
		futs.pop_front();
	}
	return 0;
}

