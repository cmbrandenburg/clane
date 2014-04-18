// vim: set noet:

#include "clane_check.hpp"
#include "../clane_http_server.hpp"

using namespace clane;

int main() {

	// no message:
	{
		http::response_record rr;
		std::ostringstream reqss;(std::ios_base::in | std::ios_base::out);
		http::request req(reqss.rdbuf());
		http::default_error_handler(rr.record(), req, http::status_code::not_found, "");
		check(rr.status == http::status_code::not_found);
		http::header_map::const_iterator p;
		check((p = rr.headers.find("content-type")) != rr.headers.end());
		check(p->second == "text/plain");
		check(rr.body.str() == "404 Not found\n");
	}

	// with message:
	{
		http::response_record rr;
		std::ostringstream reqss;(std::ios_base::in | std::ios_base::out);
		http::request req(reqss.rdbuf());
		http::default_error_handler(rr.record(), req, http::status_code::not_found, "alpha bravo");
		check(rr.status == http::status_code::not_found);
		http::header_map::const_iterator p;
		check((p = rr.headers.find("content-type")) != rr.headers.end());
		check(p->second == "text/plain");
		check(rr.body.str() == "404 Not found\nalpha bravo\n");
	}

}

