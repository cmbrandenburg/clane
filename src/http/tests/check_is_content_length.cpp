// vim: set noet:

#include "../../check/check.h"
#include "../http_consume.hpp"

using namespace clane;

int main() {

	http::header_map h;
	size_t len;

	check(!http::is_content_length(h, len));

	auto p = h.insert(http::header_map::value_type("content-length", "0"));
	check(http::is_content_length(h, len));
	check(0 == len);

	h.insert(http::header_map::value_type("transfer-encoding", "chunked"));
	check(http::is_content_length(h, len));
	check(0 == len);

	h.erase(p);
	check(!http::is_content_length(h, len));

	p = h.insert(http::header_map::value_type("content-length", "1234"));
	check(http::is_content_length(h, len));
	check(1234 == len);

	h.erase(p);

	p = h.insert(http::header_map::value_type("content-length", "invalid"));
	check(!http::is_content_length(h, len));
}

