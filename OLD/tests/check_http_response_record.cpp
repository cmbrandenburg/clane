// vim: set noet:

#include "clane_check.hpp"
#include "../clane_http_server.hpp"

using namespace clane;

void normal_handler(http::response_ostream &rs, http::request &req) {
	rs.headers.insert(http::header("content-type", "text/plain"));
	rs << "This is a response body.";
}

void status_handler(http::response_ostream &rs, http::request &req) {
	rs.status = http::status_code::not_found;
	rs.headers.insert(http::header("content-type", "text/plain"));
	rs << "Not found, duh.";
}

void no_content_handler(http::response_ostream &rs, http::request &req) {
	rs.status = http::status_code::temporary_redirect;
	rs.headers.insert(http::header("content-length", "0"));
	rs.headers.insert(http::header("location", "http://localhost/blah/blah"));
}

void late_set_handler(http::response_ostream &rs, http::request &req) {
	rs << "Not found, duh.";
	rs.status = http::status_code::not_found;
	rs.headers.insert(http::header("content-type", "text/plain"));
}

int main() {

	std::ostringstream req_body(std::ios_base::in | std::ios_base::out);
	http::request req(req_body.rdbuf());
	req.method = "GET";
	req.uri = uri::parse_uri_reference("http://localhost/alpha/bravo");
	req.headers.insert(http::header("transfer-encoding", "chunked"));
	req_body << "This is a request body.";

	// normal case:
	{
		http::response_record rr;
		normal_handler(rr.record(), req);
		check(http::status_code::ok == rr.status);
		check(rr.headers.find("content-type")->second == "text/plain");
		check(rr.body.str() == "This is a response body.");
	}

	// case: non-OK status
	{
		http::response_record rr;
		status_handler(rr.record(), req);
		check(http::status_code::not_found == rr.status);
		check(rr.headers.find("content-type")->second == "text/plain");
		check(rr.body.str() == "Not found, duh.");
	}

	// case: no body is written
	{
		http::response_record rr;
		no_content_handler(rr.record(), req);
		check(http::status_code::temporary_redirect == rr.status);
		check(rr.headers.find("content-length")->second == "0");
		check(rr.headers.find("location")->second == "http://localhost/blah/blah");
		check(rr.body.str().empty());
	}

	// case: status and headers set after body written to
	{
		http::response_record rr;
		late_set_handler(rr.record(), req);
		check(http::status_code::ok == rr.status);
		check(rr.headers.empty());
		check(rr.body.str() == "Not found, duh.");
	}

}

