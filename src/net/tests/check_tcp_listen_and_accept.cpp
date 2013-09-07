// vim: set noet:

#include "../net.h"
#include "../../check/check.h"
#include <future>

std::string client_connect(std::string const &addr);

std::string client_connect(std::string const &addr) {
	auto conn = clane::net::dial_tcp(addr, 0);
	return conn.local_addr();
}

int main(int argc, char const **argv) {
	using namespace clane::net;
	auto lis = listen_tcp("localhost:", 128, 0);
	auto client = std::async(std::launch::async, client_connect, lis.local_addr());
	std::string remote_addr;
	auto astat = lis.accept(remote_addr);
	char buf[1];
	recv_result result = astat.conn.recv(buf, sizeof(buf));
	check_eq(static_cast<size_t>(0), result.size);
	check_eq(astat.conn.remote_addr(), client.get());
	return 0;
}

