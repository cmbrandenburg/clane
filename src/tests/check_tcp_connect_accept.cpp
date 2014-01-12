// vim: set noet:

#include "check.h"
#include "../clane_inet.hpp"
#include "../clane_socket.hpp"

using namespace clane;

int main() {

	// connect & accept:
	net::socket ser, cli;
	{
		auto lis = net::listen(&net::tcp, "localhost:");
		auto conn_result = net::connect(&net::tcp, lis.local_address());
		check(net::status::ok == conn_result.stat);
		std::string accept_addr;
		auto accept_result = lis.accept(accept_addr);
		check(net::status::ok == accept_result.stat);
		ser = std::move(accept_result.sock);
		cli = std::move(conn_result.sock);
		check(accept_addr == cli.local_address());
		check(ser.remote_address() == cli.local_address());
		check(ser.local_address() == cli.remote_address());
	}

}


