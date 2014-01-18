// vim: set noet:

#include "../../check/check.h"
#include "../http_consume.hpp"
#include <cstring>

using namespace clane;

// Derive a class to access protected members.
class nop_consumer: public http::server_consumer {
public:
	using http::server_consumer::set_error;
};

int main() {

	nop_consumer cons;

	// default OK state:
	check(!cons.done());

	// setting error state:
	cons.set_error(http::status_code::not_found, "another error message");
	check(cons.done());
	check(cons.error_code() == http::status_code::not_found);
	check(!strcmp(cons.what(), "another error message"));

	// reset:
	cons.reset();
	check(!cons.done());
}

