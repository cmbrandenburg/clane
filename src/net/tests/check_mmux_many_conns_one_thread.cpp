// vim: set noet:

#include "check_mux_n_conns_n_threads.h"

int main() {
	clane::net::mmux mux;
	return check_mux_n_conns_n_threads(&mux, 10, 1);
}

