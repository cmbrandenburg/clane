// vim: set noet:

#include "../../clane_check.hpp"
#include "../clane_net_inet.hpp"
#include "../clane_net_socket.hpp"
#include <cstring>

using namespace clane;

int main() {

	std::error_code e;

	// connect & accept:
	net::socket ser, cli;
	{
		auto lis = net::listen(&net::tcp, "localhost:");
		lis.set_nonblocking();
		std::string accept_addr;
		ser = lis.accept(accept_addr, e);
		check(e == std::make_error_condition(std::errc::resource_unavailable_try_again) ||
		      e == std::make_error_condition(std::errc::operation_would_block));
		e.clear();
		cli = net::connect(&net::tcp, lis.local_address(), e);
		check(!e);
		ser = lis.accept(accept_addr, e);
		check(!e);
		ser.set_nonblocking();
		check(accept_addr == cli.local_address());
		check(ser.remote_address() == cli.local_address());
		check(ser.local_address() == cli.remote_address());
	}

	static char const *const M = "Hello, world.\n";
	char buf[strlen(M)];

	// would block receive operation:
	auto n = ser.recv(buf, sizeof(buf), e);
	check(e == std::make_error_condition(std::errc::resource_unavailable_try_again) ||
				e == std::make_error_condition(std::errc::operation_would_block));
	e.clear();

	// send from client to server:
	n = cli.send(M, strlen(M), e);
	check(!e);
	check(strlen(M) == n);

	// receive on server:
	n = ser.recv(buf, sizeof(buf), e);
	check(!e);
	check(sizeof(buf) == n);
	check(!memcmp(M, buf, sizeof(buf)));

	// would block receive operation:
	n = ser.recv(buf, sizeof(buf), e);
	check(e == std::make_error_condition(std::errc::resource_unavailable_try_again) ||
				e == std::make_error_condition(std::errc::operation_would_block));
	e.clear();

	// FIN:
	cli.fin();
	n = ser.recv(buf, sizeof(buf), e);
	check(!e);
	check(!n);
}

