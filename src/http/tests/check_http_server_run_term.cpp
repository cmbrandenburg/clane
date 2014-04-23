// vim: set noet:

#include "../clane_http_server.hpp"
#include "../../clane_check.hpp"

using namespace clane;

int main() {
	http::server s;
	std::thread thrd(&http::server::serve, &s);
	s.terminate();
	thrd.join();
}
