// vim: set noet:

#include "../../check/check.h"
#include "../http_header.hpp"
#include <cstring>

using namespace clane;

int main() {
	check("" == http::canonize_1x_header_name(""));
	check("Content-Length" == http::canonize_1x_header_name("content-length"));
	check("Content-Length" == http::canonize_1x_header_name("Content-Length"));
}

