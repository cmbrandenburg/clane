// vim: set noet:

#include "../net_mux.h"
#include "../../check/check.h"
#include <future>
#include <list>

int main() {
	clane::net::mmux mux;
	std::list<std::future<void>> futs;

	// start multiplexer threads:
	for (int i = 0; i < 10; ++i)
		futs.push_back(std::async(std::launch::async, &clane::net::mmux::run, &mux));

	// notify multiplexer to stop:
	mux.terminate();

	// wait for multiplexer threads to finish:
	while (!futs.empty()) {
		futs.front().wait();
		futs.pop_front();
	}
	return 0;
}

