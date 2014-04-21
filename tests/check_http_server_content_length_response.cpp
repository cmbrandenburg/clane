// vim: set noet:

#include "clane_check.hpp"
#include "clane_http_v1x_response.hpp"
#include "../clane_http_server.hpp"
#include "../clane_net_inet.hpp"
#include <cstring>

using namespace clane;

void handle_ok(http::response_ostream &rs, http::request &req, size_t content_len, std::string const &content) {
	std::ostringstream ss;
	ss << content_len;
	rs.headers.insert(http::header("content-length", ss.str()));
	rs.headers.insert(http::header("content-type", "text/plain"));
	rs << content;
}

int main() {

	// run server:
	http::server s;
	auto lis = net::listen(&net::tcp, "localhost:");
	std::string saddr = lis.local_address();
	s.add_listener(std::move(lis));
	std::thread thrd(&http::server::serve, &s);

	// FIXME: When Clane provides an HTTP client, use that client instead of
	// hard-coding the client-side logic here.

	{
		// Verify: Server request handler can set the Content-Length header and send
		// that exact number of bytes in the body.

		s.root_handler = std::bind(handle_ok, std::placeholders::_1, std::placeholders::_2, 5, "CHECK");

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
		check_v1x_response(resp, 1, 1, http::status_code::ok, "OK",
				http::header_map{
					http::header("content-length", "5"),
					http::header("content-type", "text/plain")
				},
				"CHECK",
				http::header_map{});
	}


	// The following test is a manual test because it verifies the server throws
	// an exception and currently there's no easy way to automate this without
	// using a child process.

	if (false) {

		// Verify: If the server request handler sets the Content-Length header and
		// sends too few or too bytes then the server throws an exception. Each case
		// requires a separate test invocation with exactly one of the following two
		// lines uncommented.

		//s.root_handler = std::bind(handle_ok, std::placeholders::_1, std::placeholders::_2, 80, "TOO FEW");
		s.root_handler = std::bind(handle_ok, std::placeholders::_1, std::placeholders::_2, 4, "TOO MANY");

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
		std::cout << resp;
	}

	// shutdown:
	s.terminate();
	thrd.join();
}

