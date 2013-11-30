// vim: set noet:

#include "../net_mux.h"
#include "../../check/check.h"
#include <future>
#include <list>

int main() {
	clane::net::smux mux;
	auto fut = std::async(std::launch::async, &clane::net::smux::run, &mux);
	mux.terminate();
	fut.wait();
	return 0;
}

