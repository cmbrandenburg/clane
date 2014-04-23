// vim: set noet:

#include "../clane_http_message.hpp"
#include "../clane_http_route.hpp"
#include "../../clane_check.hpp"
#include <sstream>

using namespace clane;

struct functor_handler { void operator()(http::response_ostream &, http::request &) {} };
static void function_handler(http::response_ostream &, http::request &) {}

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

	// empty route
	http::route r;
	check(http::route().match(req));

	// method
	check(http::route().method("GET").match(req));
	check(http::route().method("ET").match(req));
	check(!http::route().method("POST").match(req));

	// host
	check(http::route().host("foobar").match(req));
	check(http::route().host("foo").match(req));
	check(http::route().host("bar").match(req));
	check(!http::route().host("not_the_host").match(req));

	// path
	check(http::route().path("/alpha/bravo").match(req));
	check(http::route().path("/alpha").match(req));
	check(http::route().path("/bravo").match(req));
	check(!http::route().path("^/bravo").match(req));

	// header
	check(http::route().header("delta", "echo").match(req));
	check(http::route().header("delta", "fox").match(req));
	check(http::route().header("delta", "trot").match(req));
	check(!http::route().header("delta", "not_a_value").match(req));
	check(!http::route().header("not_a_name", "echo").match(req));

	// multiple items are logically anded together
	check(http::route().method("GET").host("foobar").path("/alpha").header("delta", "echo").match(req));
	check(!http::route().method("not_a_method").host("foobar").path("/alpha").header("delta", "echo").match(req));
	check(!http::route().method("GET").host("not_a_host").path("/alpha").header("delta", "echo").match(req));
	check(!http::route().method("GET").host("foobar").path("not_a_path").header("delta", "echo").match(req));
	check(!http::route().method("GET").host("foobar").path("/alpha").header("not_a_header", "echo").match(req));

	// template instantiation
	http::make_route(&function_handler);
	http::make_route(functor_handler());
}

