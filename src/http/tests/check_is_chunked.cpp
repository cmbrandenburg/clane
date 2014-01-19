// vim: set noet:

#include "../../check/check.h"
#include "../http_consume.hpp"

using namespace clane;

int main() {

	http::header_map h;

	check(!http::is_chunked(h));

	auto p = h.insert(http::header_map::value_type("transfer-encoding", "chunked"));
	check(http::is_chunked(h));

	h.insert(http::header_map::value_type("content-length", "0"));
	check(http::is_chunked(h));

	h.erase(p);
	check(!http::is_chunked(h));
}

