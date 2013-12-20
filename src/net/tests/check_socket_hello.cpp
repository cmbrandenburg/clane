// vim: set noet:

#include "check_socket_hello.h"
#include "../../check/check.h"
#include <cstring>
#include <future>
#include <iostream>

using namespace clane;

int check_socket_hello(net::protocol_family const *listen_pf, char const *saddr) {

	// listener:
	net::socket lis(listen_pf, saddr, -1);
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

	// client send:
	auto send_result = cli_sock.send("HELLO", 5, net::op_all | net::op_fin);
	check_eq(net::status::ok, send_result.stat);
	check_eq(static_cast<size_t>(5), send_result.size);

	// server receive:
	char buf[10];
	auto recv_result = serv_sock.recv(buf, 5, net::op_all);
	check_eq(net::status::ok, recv_result.stat);
	check_eq(static_cast<size_t>(5), recv_result.size);
	check_eq(0, memcmp(buf, "HELLO", 5));

	// server receive:
	recv_result = serv_sock.recv(buf, sizeof(buf));
	check_eq(net::status::ok, recv_result.stat);
	check_eq(static_cast<size_t>(0), recv_result.size);

	return 0;
}

