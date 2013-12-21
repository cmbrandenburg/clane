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
	check(lis.local_address() == cli_sock.remote_address());
	//std::cout << "local: " << cli_sock.protocol()->name() << ", " << cli_sock.local_address() << "\n";
	//std::cout << "remote: " << cli_sock.protocol()->name() << ", " << cli_sock.remote_address() << "\n";

	// server accept:
	std::string accept_addr;
	auto accept_result = lis.accept(accept_addr);
	check(accept_addr == cli_sock.local_address());
	check(net::status::ok == accept_result.stat);
	auto serv_sock = std::move(accept_result.sock);

	// client send:
	while (true) {
		static char const *m = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
		auto send_result = cli_sock.send(m, strlen(m), net::op_all | net::op_nonblock);
		check(net::status::ok == send_result.stat || net::status::would_block == send_result.stat);
		if (send_result.size < strlen(m)) {

			// send fin:
			send_result = cli_sock.send(nullptr, 0, net::op_fin);
			check(net::status::ok == send_result.stat);
			check(static_cast<size_t>(0) == send_result.size);
			break;
		}
		check(net::status::ok == send_result.stat);
		check(strlen(m) == send_result.size);
	}

	// server receive:
	{
		size_t cnt = 0;
		while (true) {
			char buf[100];
			auto recv_result = serv_sock.recv(buf, sizeof(buf));
			check(net::status::ok == recv_result.stat);
			if (0 == recv_result.size)
				break;
			for (size_t i = 0; i < recv_result.size; ++i) {
				check(buf[i] == static_cast<char>('A' + (cnt % 26)));
				++cnt;
			}
		}
	}
	

	return 0;
}

