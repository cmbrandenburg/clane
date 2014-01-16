// vim: set noet:

#include "../../check/check.h"
#include "../http_server.hpp"
#include "../../net/net_inet.hpp"
#include <cstring>
#include <iostream>
#include <thread>

using namespace clane;

static std::mutex mutex;
static std::condition_variable cond;
static bool done{};

void handle(http::oresponsestream &rs, http::request &req) {
	rs.headers.insert(std::pair<std::string, std::string>("transfer-encoding", "chunked"));
	rs.headers.insert(std::pair<std::string, std::string>("content-type", "text/plain"));
	rs << "Hello, from Clane!\n\n";
	rs << req.method << ": " << req.uri << '\n';
	for (auto h: req.headers)
		rs << h.first << ": " << h.second << '\n';
#if 0
	std::lock_guard<std::mutex> lock(mutex);
	done = true;
	cond.notify_one();
#endif
}

int main() {
	http::server s;
	s.root_handler = handle;
	s.add_listener("localhost:8080");
	std::thread thrd(&http::server::run, &s);
	// FIXME: get address from server
#if 0
	auto conn_res = net::connect(&net::tcp4, "localhost:8080");
	check(net::status::ok == conn_res.stat);
	auto cli = std::move(conn_res.sock);
#endif

#if 0
	static char const *R =
		"GET /foo/bar HTTP/1.1\r\n"
		"\r\n";
	cli.send_all(R, strlen(R));
#endif
	// 

	// shutdown:
	{
		std::unique_lock<std::mutex> lock(mutex);
		cond.wait(lock, [&]() -> bool { return done; });
	}
	s.terminate();
	thrd.join();
}

