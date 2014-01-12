// vim: set noet:

#include "../../check/check.h"
#include "../net_inet.hpp"
#include "../net_socket.hpp"
#include <cstring>

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

	// send from client to server:
	static char const *const M = "Hello, world.\n";
	auto xfer_res = cli.send(M, strlen(M));
	check(net::status::ok == xfer_res.stat);
	check(strlen(M) == xfer_res.size);

	// receive on server:
	char buf[strlen(M)];
	xfer_res = ser.recv(buf, sizeof(buf));
	check(net::status::ok == xfer_res.stat);
	check(sizeof(buf) == xfer_res.size);
	check(!memcmp(M, buf, sizeof(buf)));

	// FIN:
	cli.fin();
	xfer_res = ser.recv(buf, sizeof(buf));
	check(net::status::ok == xfer_res.stat);
	check(0 == xfer_res.size);
}


