// vim: set noet:

#include "check_mux_timeout.h"

int main() {
	clane::net::smux mux;
	return check_mux_timeout_after_attach(&mux);
}

