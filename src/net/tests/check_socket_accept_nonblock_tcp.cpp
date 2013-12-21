// vim: set noet:

#include "../net.h"
#include "../../check/check.h"
#include <cstring>
#include <future>
#include <iostream>

using namespace clane;

int main() {

	// listener:
	net::socket lis(net::tcp, "localhost:", -1);
	//std::cout << "listen: " << lis.protocol()->name() << ", " << lis.local_address() << "\n";

	// nonblocking server accept that would block:
	std::string accept_addr;
	auto accept_result = lis.accept(accept_addr, net::op_nonblock);
	check(net::status::would_block == accept_result.stat);

	// client connect:
	net::socket cli_sock(lis.protocol(), lis.local_address());
	check(lis.local_address() == cli_sock.remote_address());
	//std::cout << "local: " << cli_sock.protocol()->name() << ", " << cli_sock.local_address() << "\n";
	//std::cout << "remote: " << cli_sock.protocol()->name() << ", " << cli_sock.remote_address() << "\n";

	// nonblocking server accept that succeeds:
	do {
		accept_result = lis.accept(accept_addr, net::op_nonblock);
	} while (net::status::would_block == accept_result.stat);
	check(accept_addr == cli_sock.local_address());
	check(net::status::ok == accept_result.stat);
	auto serv_sock = std::move(accept_result.sock);

	// client send:
	auto send_result = cli_sock.send("HELLO", 5, net::op_all | net::op_fin);
	check(net::status::ok == send_result.stat);
	check(static_cast<size_t>(5) == send_result.size);

	// server receive:
	char buf[10];
	auto recv_result = serv_sock.recv(buf, 5, net::op_all);
	check(net::status::ok == recv_result.stat);
	check(static_cast<size_t>(5) == recv_result.size);
	check(0 == memcmp(buf, "HELLO", 5));

	// server receive:
	recv_result = serv_sock.recv(buf, sizeof(buf));
	check(net::status::ok == recv_result.stat);
	check(static_cast<size_t>(0) == recv_result.size);

	return 0;
}

