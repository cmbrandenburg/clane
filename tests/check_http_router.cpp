// vim: set noet:

#include "clane_check.hpp"
#include "../clane_http_server.hpp"
#include "../clane_http_route.hpp"

using namespace clane;

static void handler(http::response_ostream &rs, http::request &req, char const *s) {
	rs << "check: " << s;
}

static std::function<void(http::response_ostream &, http::request &)> make_handler(char const *s) {
	return std::bind(handler, std::placeholders::_1, std::placeholders::_2, s);
}

int main() {

	std::ostringstream reqss(std::ios_base::in | std::ios_base::out);
	http::request req(reqss.rdbuf());
	req.method = "GET";
	req.uri = uri::parse_uri_reference("/alpha/bravo?charlie");
	req.major_version = 1;
	req.minor_version = 1;
	req.headers.insert(http::header("delta", "echo"));
	req.headers.insert(http::header("delta", "foxtrot"));
	req.headers.insert(http::header("golf", "hotel"));
	req.headers.insert(http::header("host", "foobar"));

	http::router r;

	// empty router -> not found error
	{
		http::response_record rr;
		r(rr.record(), req);
		check(rr.status == http::status_code::not_found);
	}

	// no match -> not found error
	{
		r.new_route(make_handler("ALPHA")).method("POST");
		http::response_record rr;
		r(rr.record(), req);
		check(rr.status == http::status_code::not_found);
	}

	// match -> dispatches to handler
	{
		r.new_route(make_handler("BRAVO")).method("GET");
		http::response_record rr;
		r(rr.record(), req);
		check(rr.status == http::status_code::ok);
		check(rr.body.str() == "check: BRAVO");
	}

	// clear router
	{
		r.clear();
		http::response_record rr;
		r(rr.record(), req);
		check(rr.status == http::status_code::not_found);
	}

}

