// vim: set noet:

#include "../net.h"
#include "../../check/check.h"
#include <future>

void client_connect(std::string const &addr);

void client_connect(std::string const &addr) {
	auto conn = clane::net::dial_tcp(addr, 0);
	// (Close socket to cause shutdown.)
}

int main(int argc, char const **argv) {
	using namespace clane::net;

	// accept:
	auto lis = listen_tcp("localhost:", 128, 0);
	auto client = std::async(std::launch::async, client_connect, lis.local_addr());
	auto astat = lis.accept();

	// receive:
	char buf[100];
	auto recv_status = astat.conn.recv(buf, sizeof(buf));
	check_false(recv_status.reset);
	check_true(recv_status.shutdown);
	check_eq(static_cast<size_t>(0), recv_status.size);

	client.wait();
	return 0;
}

