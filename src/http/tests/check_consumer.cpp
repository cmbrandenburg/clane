// vim: set noet:

#include "../http_consumer.h"
#include "../../check/check.h"
#include <cstring>

using namespace clane;

class nop_consumer: public http::consumer {
public:
	using http::consumer::increase_length;
	using http::consumer::set_error;
};

int main() {

	nop_consumer cons;

	// default OK state:
	check(cons);
	check(0 == cons.length());

	check(cons.increase_length(1234));
	check(1234 == cons.length());

	// setting error state:
	cons.set_error(http::status_code::not_found, "my error message");
	check(!cons);
	check(cons.error_code() == http::status_code::not_found);
	check(!strcmp(cons.what(), "my error message"));

	// reset:
	cons.reset();
	check(cons);
	check(0 == cons.length());

	// length limits:
	cons.set_length_limit(100);
	check(cons.increase_length(100));
	cons.reset();
	check(!cons.increase_length(101));
	cons.reset();
	check(cons.increase_length(99));
	check(cons.increase_length(1));
	check(!cons.increase_length(1));

	return 0;
}

