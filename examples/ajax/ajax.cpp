// vim: set noet:

#include "../../include/clane.hpp"
#include <fstream>
#include <thread>

static void count(clane::http::response_ostream &rs, clane::http::request &req) {
	static std::mutex mutex;
	static int cnt;
	int n;
	{
		std::unique_lock<std::mutex> lock(mutex);
		n = ++cnt;
	}
	rs << "{\"n\": " << n << "}\n"; // JSON
		std::cout << "count -> " << n << std::endl;
}

// Program invocation:
//
//  ./hello          Listen on port 8080 (default port)
//  ./hello :1234    Listen on port 1234
//
int main(int argc, char **argv) {

	char const *saddr = argc == 1 ? ":8080" : argv[1];

	// Sanity check to make sure program is run in correct directory.
	{
		std::ifstream f("index.html");
		if (!f) {
			std::cerr << "cannot find index.html--is the current working directory correct?\n";
			return 1;
		}
	}

	// Set up a server instance.
	//
	// For all GET requests with URI path beginning with "/static", strip that
	// prefix and serve as a file.
	//
	// For GET requests "/", serve "index.html".
	//
	// For POST requests to "/count", call the count() method.
	//
	// (All other requests generate a 404 Not Found error.)

	clane::http::router router;
	router.new_route(clane::http::make_prefix_stripper("/static", clane::http::file_server("."))).
		method("^GET$").path("^/static/");
	router.new_route(clane::http::make_prefix_stripper("/", clane::http::file_server("index.html"))).
		method("^GET$").path("^/$");
	router.new_route(&count).method("^POST$").path("^/count$");
	auto s = clane::http::make_server(std::move(router));
	s.add_listener(saddr);

	// Run the server. This function should never return because this program
	// doesn't call the server instance's terminate() method.
	s.serve();
}

