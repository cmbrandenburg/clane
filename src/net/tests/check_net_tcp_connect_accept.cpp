// vim: set noet:

#include "../../clane_check.hpp"
#include "../clane_net_inet.hpp"
#include "../clane_net_socket.hpp"
#include <cstring>

using namespace clane;

int main() {

	std::error_code e;

	// connect & accept:
	net::socket ser, cli;
	{
		auto lis = net::listen(&net::tcp, "localhost:");
		cli = net::connect(&net::tcp, lis.local_address(), e);
		check(!e);
		std::string accept_addr;
		ser = lis.accept(accept_addr, e);
		check(!e);
		check(accept_addr == cli.local_address());
		check(ser.remote_address() == cli.local_address());
		check(ser.local_address() == cli.remote_address());
	}

	// send from client to server:
	static char const *const M = "Hello, world.\n";
	auto n = cli.send(M, strlen(M), e);
	check(!e);
	check(strlen(M) == n);

	// receive on server:
	char buf[strlen(M)];
	n = ser.recv(buf, sizeof(buf), e);
	check(!e);
	check(sizeof(buf) == n);
	check(!memcmp(M, buf, sizeof(buf)));

	// FIN:
	cli.fin();
	n = ser.recv(buf, sizeof(buf), e);
	check(!e);
	check(!n);
}

