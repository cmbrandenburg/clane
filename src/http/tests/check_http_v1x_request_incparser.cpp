// vim: set noet:

#include "../clane_http_parse.hpp"
#include "../../clane_check.hpp"
#include <cstring>

using namespace clane;

void check_ok(char const *content, char const *exp_method, char const *exp_uri, int exp_major, int exp_minor,
	   	http::header_map const &exp_hdrs, std::string const &exp_body, http::header_map const &exp_trls) {

	std::string got_body;
	http::v1x_request_incparser pars;

	// as few passes as possible:
	pars.reset();
	size_t i = 0;
	bool got_hdrs = false;
	while (i < std::strlen(content)) {
		size_t stat = pars.parse_some(content+i, content+std::strlen(content));
		check(pars.error != stat);
		if (pars.got_headers())
			if (!got_hdrs) {
				got_hdrs = true;
				check(0 == pars.size());
			}
			got_body.append(content+i+pars.offset(), pars.size());
		i += stat;
	}
	check(!pars);
	check(pars.got_headers());
	check(pars.method() == exp_method);
	check(pars.uri().string() == exp_uri);
	check(pars.major_version() == exp_major);
	check(pars.minor_version() == exp_minor);
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
	check(pars.method() == exp_method);
	check(pars.uri().string() == exp_uri);
	check(pars.major_version() == exp_major);
	check(pars.minor_version() == exp_minor);
	check(pars.headers() == exp_hdrs);
	check(got_body == exp_body);
	check(pars.trailers() == exp_trls);
}

void check_nok(char const *ok, char const *bad, http::status_code exp_error_code) {

	http::v1x_request_incparser pars;

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
		check(exp_error_code == pars.status());
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
		check(exp_error_code == pars.status());
	}
}

int main() {

	// no body:
	check_ok("GET / HTTP/1.1\r\n\r\n", "GET", "/", 1, 1, http::header_map{}, "", http::header_map{});

	// no body, chunked:
	check_ok("GET / HTTP/1.1\r\nTransfer-Encoding: chunked\r\n\r\n0\r\n\r\n",
		 	"GET", "/", 1, 1, http::header_map{http::header("transfer-encoding", "chunked")}, "", http::header_map{});

	// no body, content-length:
	check_ok("GET / HTTP/1.1\r\nContent-Length: 0\r\n\r\n",
		 	"GET", "/", 1, 1, http::header_map{http::header("content-length", "0")}, "", http::header_map{});

	// some body, chunked:
	check_ok("GET / HTTP/1.1\r\nTransfer-Encoding: chunked\r\n\r\nd\r\nHello, world.\r\n0\r\n\r\n",
		 	"GET", "/", 1, 1, http::header_map{http::header("transfer-encoding", "chunked")},
		 	"Hello, world.", http::header_map{});

	// some body, chunked with trailers:
	check_ok("GET / HTTP/1.1\r\nTransfer-Encoding: chunked\r\n\r\nd\r\nHello, world.\r\n0\r\nContent-Type: text/plain\r\n\r\n",
		 	"GET", "/", 1, 1, http::header_map{http::header("transfer-encoding", "chunked")},
		 	"Hello, world.", http::header_map{http::header("content-type", "text/plain")});

	// some body, content-length:
	check_ok("GET / HTTP/1.1\r\nContent-Length: 13\r\n\r\nHello, world.",
		 	"GET", "/", 1, 1, http::header_map{http::header("content-length", "13")},
		 	"Hello, world.", http::header_map{});

	// error: bad request line
	check_nok("GET / ", "\r\nContent-Length: 13\r\n\r\nHello, world.", http::status_code::bad_request);

	// error: bad header
	check_nok("GET / HTTP/1.1\r\nCont", "\rent-Length: 13\r\n\r\nHello, world.", http::status_code::bad_request);

	// error: bad trailer
	check_nok("GET / HTTP/1.1\r\nTransfer-Encoding: chunked\r\n\r\nd\r\nHello, world.\r\n0\r\nCont",
		 	"\rent-Type: text/plain\r\n\r\n", http::status_code::bad_request);
}

