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
	rs.headers.insert(std::pair<std::string, std::string>("content-type", "text/plain"));
	rs << "Hello, from Clane!\n";
	std::lock_guard<std::mutex> lock(mutex);
	done = true;
	cond.notify_one();
}

int main() {
	return 77;

	// run server:
	// FIXME: use variable port number
	http::server s;
	s.root_handler = handle;
	s.add_listener(":8080");
	std::thread thrd(&http::server::run, &s);
	// FIXME: get address from server

	// connect to server:
	auto conn_res = net::connect(&net::tcp4, "localhost:8080");
	check(net::status::ok == conn_res.stat);
	auto cli = std::move(conn_res.sock);

	// send request:
	static char const *R =
		"GET /foo/bar HTTP/1.1\r\n"
		"\r\n";
	auto xfer_res = cli.send(R, strlen(R), net::all);
	check(net::status::ok == xfer_res.stat);
	cli.fin();

	// wait for response:
	{
		std::unique_lock<std::mutex> lock(mutex);
		cond.wait(lock, [&]() -> bool { return done; });
	}

	// check response:
	std::string resp;
	while (true) {
		char buf[100];
		xfer_res = cli.recv(buf, sizeof(buf));
		check(net::status::ok == xfer_res.stat);
		if (!xfer_res.size)
			break;
		resp += std::string(buf, xfer_res.size);
	}
	check(resp == "HTTP/1.1 200 OK\r\n"
			"Content-Type: text/plain\r\n"
			"Transfer-Encoding: chunked\r\n"
			"\r\n"
			"13\r\n"
			"Hello, from Clane!\n"
			"\r\n"
			"0\r\n"
			"\r\n");

	// shutdown:
	s.terminate();
	thrd.join();
}

