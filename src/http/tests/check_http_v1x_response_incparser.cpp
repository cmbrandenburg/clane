// vim: set noet:

#include "../clane_http_parse.hpp"
#include "../../clane_check.hpp"
#include <cstring>

using namespace clane;

void check_ok(char const *content, int exp_major, int exp_minor, http::status_code exp_stat, char const *exp_reason,
		http::header_map const &exp_hdrs, std::string const &exp_body, http::header_map const &exp_trls) {

	std::string got_body;
	http::v1x_response_incparser pars;

	// as few passes as possible:
	pars.reset();
	size_t i = 0;
	while (i < std::strlen(content)) {
		size_t stat = pars.parse_some(content+i, content+std::strlen(content));
		check(pars.error != stat);
		if (pars.got_headers())
			got_body.append(content+i+pars.offset(), pars.size());
		i += stat;
	}
	check(!pars);
	check(pars.got_headers());
	check(pars.major_version() == exp_major);
	check(pars.minor_version() == exp_minor);
	check(pars.status() == exp_stat);
	check(pars.reason() == exp_reason);
	check(pars.headers() == exp_hdrs);
	check(got_body == exp_body);
	check(pars.trailers() == exp_trls);

	// byte-by-byte:
	got_body.clear();
	pars.reset();
	for (size_t i = 0; i < std::strlen(content)-1; ++i) {
		check(0 == pars.parse_some(content+i, content+i));
		check(pars);
		check(1 == pars.parse_some(content+i, content+i+1));
		check(pars);
		if (pars.got_headers())
			got_body.append(content+i+pars.offset(), pars.size());
	}
	check(0 == pars.parse_some(content+std::strlen(content)-1, content+std::strlen(content)-1));
	check(pars);
	check(1 == pars.parse_some(content+std::strlen(content)-1, content+std::strlen(content)));
	got_body.append(content+std::strlen(content)-1+pars.offset(), pars.size());
	check(!pars);
	check(pars.got_headers());
	check(pars.major_version() == exp_major);
	check(pars.minor_version() == exp_minor);
	check(pars.status() == exp_stat);
	check(pars.reason() == exp_reason);
	check(pars.headers() == exp_hdrs);
	check(got_body == exp_body);
	check(pars.trailers() == exp_trls);
}

void check_nok(char const *ok, char const *bad) {

	http::v1x_response_incparser pars;

	// as few passes as possible:
	{
		std::string full(ok);
		full.append(bad);
		pars.reset();
		size_t i = 0;
		while (i < full.size()) {
			size_t stat = pars.parse_some(full.data()+i, full.data()+full.size());
			if (pars.error == stat)
				break;
			i += stat;
		}
		check(!pars);
	}

	// byte-by-byte:
	{
		pars.reset();
		for (size_t i = 0; i < std::strlen(ok); ++i) {
			check(0 == pars.parse_some(ok+i, ok+i));
			check(pars);
			check(1 == pars.parse_some(ok+i, ok+i+1));
			check(pars);
		}
		check(0 == pars.parse_some(bad, bad));
		check(pars);
		check(pars.error == pars.parse_some(bad, bad+1));
		check(!pars);
	}
}

int main() {

	// no body, chunked:
	check_ok("HTTP/1.1 200 OK\r\nTransfer-Encoding: chunked\r\n\r\n0\r\n\r\n", 1, 1, http::status_code::ok, "OK",
		 	http::header_map{http::header("transfer-encoding", "chunked")}, "", http::header_map{});

	// no body, content-length:
	check_ok("HTTP/1.1 200 OK\r\nContent-Length: 0\r\n\r\n", 1, 1, http::status_code::ok, "OK",
		 	http::header_map{http::header("content-length", "0")}, "", http::header_map{});

	// some body, chunked:
	check_ok("HTTP/1.1 200 OK\r\nTransfer-Encoding: chunked\r\n\r\nd\r\nHello, world.\r\n0\r\n\r\n",
		 	1, 1, http::status_code::ok, "OK",
		 	http::header_map{http::header("transfer-encoding", "chunked")}, "Hello, world.", http::header_map{});

	// some body, chunked with trailers:
	check_ok("HTTP/1.1 200 OK\r\nTransfer-Encoding: chunked\r\n\r\nd\r\nHello, world.\r\n0\r\nContent-Type: text/plain\r\n\r\n",
		 	1, 1, http::status_code::ok, "OK",
		 	http::header_map{http::header("transfer-encoding", "chunked")}, "Hello, world.",
		 	http::header_map{http::header("content-type", "text/plain")});

	// some body, content-length:
	check_ok("HTTP/1.1 200 OK\r\nContent-Length: 13\r\n\r\nHello, world.",
		 	1, 1, http::status_code::ok, "OK",
		 	http::header_map{http::header("content-length", "13")}, "Hello, world.", http::header_map{});

	// error: bad status line
	check_nok("HTTP/invalid", " 200 OK\r\nContent-Length: 13\r\n\r\nHello, world.");

	// error: bad header
	check_nok("HTTP/1.1 200 OK\r\nCont", "\rent-Length: 13\r\n\r\nHello, world.");

	// error: bad trailer
	check_nok("HTTP/1.1 200 OK\r\nTransfer-Encoding: chunked\r\n\r\nd\r\nHello, world.\r\n0\r\nCont",
			"\rent-Type: text/plain\r\n\r\n");

}

