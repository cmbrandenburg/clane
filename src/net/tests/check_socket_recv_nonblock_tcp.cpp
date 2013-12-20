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

	// client connect:
	net::socket cli_sock(lis.protocol(), lis.local_address());
	check_eq(lis.local_address(), cli_sock.remote_address());
	//std::cout << "local: " << cli_sock.protocol()->name() << ", " << cli_sock.local_address() << "\n";
	//std::cout << "remote: " << cli_sock.protocol()->name() << ", " << cli_sock.remote_address() << "\n";

	// server accept:
	std::string accept_addr;
	auto accept_result = lis.accept(accept_addr);
	check_eq(accept_addr, cli_sock.local_address());
	check_eq(net::status::ok, accept_result.stat);
	auto serv_sock = std::move(accept_result.sock);

	// nonblocking server receive that would block:
	char buf[10];
	auto recv_result = serv_sock.recv(buf, 5, net::op_nonblock);
	check_eq(net::status::would_block, recv_result.stat);
	check_eq(static_cast<size_t>(0), recv_result.size);

	// nonblocking server receive that would block, with "all" flag:
	recv_result = serv_sock.recv(buf, 5, net::op_nonblock | net::op_all);
	check_eq(net::status::would_block, recv_result.stat);
	check_eq(static_cast<size_t>(0), recv_result.size);

	// client send:
	auto send_result = cli_sock.send("HELLO", 5, net::op_all | net::op_fin);
	check_eq(net::status::ok, send_result.stat);
	check_eq(static_cast<size_t>(5), send_result.size);

	// nonblocking server receive that succeeds:
	size_t total = 0;
	do {
		recv_result = serv_sock.recv(buf+total, 5-total, net::op_nonblock);
		check_eq(net::status::ok, recv_result.stat);
		check_lteq(recv_result.size, static_cast<size_t>(5-total));
		total += recv_result.size;
	} while (total < 5);
	check_eq(0, memcmp(buf, "HELLO", 5));

	// nonblocking server receive that receives FIN:
	recv_result = serv_sock.recv(buf, sizeof(buf), net::op_nonblock);
	check_eq(net::status::ok, recv_result.stat);
	check_eq(static_cast<size_t>(0), recv_result.size);

	// nonblocking server receive that receives FIN, with "all" flag:
	recv_result = serv_sock.recv(buf, sizeof(buf), net::op_nonblock | net::op_all);
	check_eq(net::status::ok, recv_result.stat);
	check_eq(static_cast<size_t>(0), recv_result.size);

	return 0;
}

