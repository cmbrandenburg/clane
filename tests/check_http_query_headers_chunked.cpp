// vim: set noet:

#include "clane_check.hpp"
#include "../clane_http_parse.hpp"

using namespace clane;

int main() {

	http::header_map h;

	check(!http::query_headers_chunked(h));

	auto p = h.insert(http::header_map::value_type("transfer-encoding", "chunked"));
	check(http::query_headers_chunked(h));

	h.insert(http::header_map::value_type("content-length", "0"));
	check(http::query_headers_chunked(h));

	h.erase(p);
	check(!http::query_headers_chunked(h));

	// case-insensitivity
	p = h.insert(http::header_map::value_type("Transfer-Encoding", "chunked"));
	check(http::query_headers_chunked(h));
}

