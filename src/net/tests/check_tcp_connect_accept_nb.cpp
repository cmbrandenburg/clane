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
		lis.set_nonblocking();
		std::string accept_addr;
		auto accept_res = lis.accept(accept_addr);
		check(net::status::would_block == accept_res.stat);
		auto conn_res = net::connect(&net::tcp, lis.local_address());
		check(net::status::ok == conn_res.stat);
		accept_res = lis.accept(accept_addr);
		check(net::status::ok == accept_res.stat);
		ser = std::move(accept_res.sock);
		ser.set_nonblocking();
		cli = std::move(conn_res.sock);
		check(accept_addr == cli.local_address());
		check(ser.remote_address() == cli.local_address());
		check(ser.local_address() == cli.remote_address());
	}

	static char const *const M = "Hello, world.\n";
	char buf[strlen(M)];

	// would block receive operation:
	auto xfer_res = ser.recv(buf, sizeof(buf));
	check(net::status::would_block == xfer_res.stat);

	// send from client to server:
	xfer_res = cli.send(M, strlen(M));
	check(net::status::ok == xfer_res.stat);
	check(strlen(M) == xfer_res.size);

	// receive on server:
	xfer_res = ser.recv(buf, sizeof(buf));
	check(net::status::ok == xfer_res.stat);
	check(sizeof(buf) == xfer_res.size);
	check(!memcmp(M, buf, sizeof(buf)));

	// would block receive operation:
	xfer_res = ser.recv(buf, sizeof(buf));
	check(net::status::would_block == xfer_res.stat);

	// FIN:
	cli.fin();
	xfer_res = ser.recv(buf, sizeof(buf));
	check(net::status::ok == xfer_res.stat);
	check(0 == xfer_res.size);
}

