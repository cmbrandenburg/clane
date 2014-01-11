// vim: set noet:

#include "check.h"
#include "../clane_inet.hpp"
#include "../clane_socket.hpp"

using namespace clane;

int main() {
	auto lis = net::listen(&net::tcp, "localhost:");
	auto conn_result = net::connect(&net::tcp, lis.local_address());
	check(net::status::ok == conn_result.stat);
}


