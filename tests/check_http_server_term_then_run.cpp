// vim: set noet:

#include "clane_check.hpp"
#include "../clane_http_server.hpp"

using namespace clane;

int main() {
	http::server s;
	s.terminate();
	s.serve();
}

