// vim: set noet:

#include "check_socket_hello.h"
#include "../net_inet.h"

int main() {
	return check_socket_hello(clane::net::tcp, "localhost:");
}

