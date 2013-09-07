// vim: set noet:

#include "../net_mux.h"
#include "../../check/check.h"
#include <future>
#include <list>

int main() {
	clane::net::unique_mux mux;
	auto fut = std::async(std::launch::async, &clane::net::unique_mux::run, &mux);
	mux.cancel();
	fut.wait();
	return 0;
}

