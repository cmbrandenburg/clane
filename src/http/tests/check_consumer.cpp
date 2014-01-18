// vim: set noet:

#include "../../check/check.h"
#include "../http_consume.hpp"
#include <cstring>

using namespace clane;

// Derive a class to access protected members.
class nop_consumer: public http::consumer {
public:
	using http::consumer::increase_length;
	using http::consumer::set_error;
};

int main() {

	nop_consumer cons;

	// default OK state:
	check(!cons.done());

	// setting error state:
	cons.set_error("another error message");
	check(cons.done());
	check(!strcmp(cons.what(), "another error message"));

	// reset:
	cons.reset();
	check(!cons.done());

	// increase length without length limit:
	check(cons.increase_length(1234));

	// length limits:
	cons.reset();
	cons.set_length_limit(100);
	check(cons.increase_length(100));
	cons.reset();
	check(!cons.increase_length(101));
	cons.reset();
	check(cons.increase_length(99));
	check(cons.increase_length(1));
	check(!cons.increase_length(1));
}

