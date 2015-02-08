// vim: set noet:

#include "clane_check.hpp"
#include "../clane_http_server.hpp"
#include "../include/clane_http_prefix_stripper.hpp"
#include <cstring>

using namespace clane;

static std::string got;

static void handler1(http::response_ostream &rs, http::request &req, std::string &ostr) {
	ostr = req.uri.path;
}

static void handler2(http::response_ostream &rs, http::request &req) {
	got = req.uri.path;
}

template <typename PrefixStripper> void check_ok(PrefixStripper &&ps) {
	got.clear();
	std::ostringstream reqss(std::ios_base::in | std::ios_base::out);
	http::request req(reqss.rdbuf());
	req.uri = uri::parse_uri_reference("/alpha/bravo/charlie.html");
	http::response_record rr;
	ps(rr.record(), req);
	check(got == "/bravo/charlie.html");
}

int main() {
	check_ok(http::prefix_stripper("/alpha", std::bind(handler1, std::placeholders::_1, std::placeholders::_2, std::ref(got))));
	check_ok(http::make_prefix_stripper("/alpha", std::bind(handler1, std::placeholders::_1, std::placeholders::_2, std::ref(got))));
	check_ok(http::make_prefix_stripper("/alpha", &handler2));
}

