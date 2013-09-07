// vim: set noet:

#include "../net.h"
#include "../../check/check.h"
#include <future>

void client_connect(std::string const &addr);
std::string const client_msg = "Hello, server.";
std::string const server_msg = "Hello, client.";

void client_connect(std::string const &addr) {
	auto conn = clane::net::dial_tcp(addr, 0);

	// send:
	auto send_status = conn.send(client_msg.c_str(), client_msg.size());
	check_false(send_status.reset);
	check_eq(client_msg.size(), send_status.size);

	// receive:
	char buf[100];
	auto recv_status = conn.recv(buf, sizeof(buf));
	check_false(recv_status.reset);
	check_false(recv_status.shutdown);
	check_eq(server_msg.size(), recv_status.size);
	check_eq(server_msg, std::string(buf));
}

int main(int argc, char const **argv) {
	using namespace clane::net;

	// accept:
	auto lis = listen_tcp("localhost:", 128, 0);
	auto client = std::async(std::launch::async, client_connect, lis.local_addr());
	std::string remote_addr;
	auto astat = lis.accept(remote_addr);

	// receive:
	char buf[100];
	auto recv_status = astat.conn.recv(buf, sizeof(buf));
	check_false(recv_status.reset);
	check_false(recv_status.shutdown);
	check_eq(client_msg.size(), recv_status.size);
	check_eq(client_msg, std::string(buf));

	// send:
	auto send_status = astat.conn.send(server_msg.c_str(), server_msg.size());
	check_false(send_status.reset);
	check_eq(server_msg.size(), send_status.size);

	// wait for hangup:
	recv_status = astat.conn.recv(buf, sizeof(buf));
	check_false(recv_status.reset);
	check_true(recv_status.shutdown);
	check_eq(static_cast<size_t>(0), recv_status.size);
	client.wait();
	return 0;
}

