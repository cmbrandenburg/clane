// vim: set noet:

#include "check_mux_run_terminate.h"
#include "../../check/check.h"
#include <future>

int check_mux_run_terminate(clane::net::mux *mux) {
	auto mux_fut = std::async(std::launch::async, &clane::net::mux::run, mux);
	mux->terminate();
	mux_fut.wait();
	return 0;
}

