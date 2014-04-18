// vim: set noet:

#include "clane_check.hpp"
#include "../clane_http_parse.hpp"

using namespace clane;

int main() {

	http::header_map h;
	size_t len;

	check(!http::query_headers_content_length(h, len));

	auto p = h.insert(http::header_map::value_type("content-length", "0"));
	check(http::query_headers_content_length(h, len));
	check(0 == len);

	h.insert(http::header_map::value_type("transfer-encoding", "chunked"));
	check(http::query_headers_content_length(h, len));
	check(0 == len);

	h.erase(p);
	check(!http::query_headers_content_length(h, len));

	p = h.insert(http::header_map::value_type("content-length", "1234"));
	check(http::query_headers_content_length(h, len));
	check(1234 == len);

	h.erase(p);

	p = h.insert(http::header_map::value_type("content-length", "invalid"));
	check(!http::query_headers_content_length(h, len));
}

