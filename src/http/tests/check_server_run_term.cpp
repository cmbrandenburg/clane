// vim: set noet:

#include "../http_server.hpp"
#include <thread>

using namespace clane;

int main() {
	http::server s;
	std::thread thrd(&http::server::run, &s);
	s.terminate();
	thrd.join();
}

