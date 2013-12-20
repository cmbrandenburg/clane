// vim: set noet:

#include "check_mux_run_terminate.h"

int main() {
	clane::net::smux mux;
	return check_mux_run_terminate(&mux);
}

