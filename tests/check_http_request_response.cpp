// vim: set noet:

#include "clane_check.hpp"
#include "clane_http_v1x_response.hpp"
#include "../clane_http_server.hpp"
#include "../clane_net_inet.hpp"
#include <cstring>

using namespace clane;

static std::mutex mutex;
static std::condition_variable cond;
static bool done{};

void handle(http::response_ostream &rs, http::request &req) {
	rs.headers.insert(std::pair<std::string, std::string>("content-type", "text/plain"));
	rs << "Hello, from Clane!\n";
	std::lock_guard<std::mutex> lock(mutex);
	done = true;
	cond.notify_one();
}

int main() {

	// run server:
	http::server s;
	s.root_handler = handle;
	auto lis = net::listen(&net::tcp, "localhost:");
	std::string saddr = lis.local_address();
	s.add_listener(std::move(lis));
	std::thread thrd(&http::server::serve, &s);
	// FIXME: get address from server

	// connect to server:
	std::error_code e;
	auto cli = net::connect(&net::tcp4, saddr, e);
	check(!e);

	// send request:
	static char const *R =
		"GET /foo/bar HTTP/1.1\r\n"
		"\r\n";
	auto xstat = cli.send(R, std::strlen(R), net::all, e);
	check(!e);
	cli.fin();

	// wait for response:
	{
		std::unique_lock<std::mutex> lock(mutex);
		while (!done)
			cond.wait(lock);
	}

	// check response:
	std::string resp;
	while (true) {
		char buf[100];
		xstat = cli.recv(buf, sizeof(buf), e);
		check(!e);
		if (!xstat)
			break;
		resp += std::string(buf, xstat);
	}
	check_v1x_response(resp, 1, 1, http::status_code::ok, "OK",
			http::header_map{http::header("content-type", "text/plain"), http::header("transfer-encoding", "chunked")},
			"Hello, from Clane!\n",
			http::header_map{});

	// shutdown:
	s.terminate();
	thrd.join();
}

