// vim: set noet:

#include "clane_http_v1x_response.hpp"
#include "../clane_http_server.hpp"
#include "../../clane_check.hpp"
#include <cstring>

using namespace clane;

static std::mutex mutex;
static http::status_code exp_scode;
static std::string saddr;

static void empty_response(http::response_ostream &rs, http::request &req) {
	std::unique_lock<std::mutex> lock(mutex);
	rs.status = exp_scode;
}

static void content_type(http::response_ostream &rs, http::request &req) {
	rs.headers.insert(http::header("content-type", "text/html"));
	std::unique_lock<std::mutex> lock(mutex);
	rs.status = exp_scode;
}

static void nonempty(http::response_ostream &rs, http::request &req) {
	{
		std::unique_lock<std::mutex> lock(mutex);
		rs.status = exp_scode;
	}
	rs << "CHECK";
}

static void flushed(http::response_ostream &rs, http::request &req) {
	{
		std::unique_lock<std::mutex> lock(mutex);
		rs.status = exp_scode;
	}
	rs.flush();
}

static void check_ok(bool reason_added, http::status_code scode, std::string const &exp_body = "") {

	// FIXME: When Clane provides an HTTP client, use that client instead of
	// hard-coding the client-side logic here.

	{
		std::unique_lock<std::mutex> lock(mutex);
		exp_scode = scode;
	}

	// connect to server:
	std::error_code e;
	auto cli = net::connect(&net::tcp4, saddr, e);
	check(!e);

	// send request:
	static char const *R =
		"GET / HTTP/1.1\r\n"
		"\r\n";
	auto xstat = cli.send(R, std::strlen(R), net::all, e);
	check(!e);
	cli.fin();

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

	if (reason_added) {
		check_v1x_response(resp, 1, 1, scode, what(scode),
				http::header_map{http::header("content-type", "text/plain"), http::header("transfer-encoding", "chunked")},
				std::string(what(scode)) + "\n",
				http::header_map{});
	} else {
		check_v1x_response(resp, 1, 1, scode, what(scode),
				http::header_map{http::header("transfer-encoding", "chunked")},
				exp_body,
				http::header_map{});
	}
}

int main() {

	// This program tests that the server adds a reason phrase to the response
	// body for any response with a 4xx or 5xx status code and whose response body
	// would otherwise be empty.

	// run server:
	http::server s;
	s.root_handler = empty_response;
	auto lis = net::listen(&net::tcp, "localhost:");
	saddr = lis.local_address();
	s.add_listener(std::move(lis));
	std::thread thrd(&http::server::serve, &s);

	// 1xx: unmodified
	check_ok(false, http::status_code::cont);

	// 2xx: unmodified
	check_ok(false, http::status_code::ok);

	// 3xx: unmodified
	check_ok(false, http::status_code::multiple_choices);

	// 4xx: reason phrase added
	check_ok(true, http::status_code::not_found);

	// 5xx: reason phrase added
	check_ok(true, http::status_code::internal_server_error);

	// Even if the root handler sets the "content-type" header, the server will
	// modify the body for an error response whose body is empty and not flushed.
	s.root_handler = content_type;
	check_ok(true, http::status_code::not_found);

	// The server doesn't modify a nonempty response.
	s.root_handler = nonempty;
	check_ok(false, http::status_code::not_found, "CHECK");

	// The server doesn't modify an empty, flushed response.
	s.root_handler = flushed;
	check_ok(false, http::status_code::not_found);

	// shutdown:
	s.terminate();
	thrd.join();
}

