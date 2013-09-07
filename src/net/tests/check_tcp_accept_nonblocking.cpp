// vim: set noet:

#include "../net.h"
#include "../../check/check.h"
#include <future>

using namespace clane::net;

int main(int argc, char const **argv) {
	auto lis = listen_tcp("localhost:", 16, 0);
	lis.set_nonblocking();
	auto astat = lis.accept();
	check_false(astat.aborted);
	check_false(astat.conn);
	return 0;
}

