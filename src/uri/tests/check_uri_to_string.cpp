// vim: set noet:

#include "check/check.h"

#define check_ok(in, exp) \
	do { \
		uri::uri u = uri::parse_uri_reference(in); \
		std::string out = u.string(); \
		check(out == exp); \
	} while (false)

int main() {

#if 0
	// empty:
	check_ok("", "");

	// all fields:
	check_ok("http://john_doe@example.com:80/alpha/bravo?charlie=delta#echo",
	         "http://john_doe@example.com:80/alpha/bravo?charlie=delta#echo");

	// all fields, with percent-encoding:
	check_ok("http://%6aohn_%64oe@ex%61mple.com:80/al%70ha/%62ravo?%63harlie=%64elta#e%63h%6f",
	         "http://john_doe@example.com:80/alpha/bravo?charlie=delta#echo");

	// no scheme:
	check_ok("//john_doe@example.com:80/alpha/bravo?charlie=delta#echo",
	         "//john_doe@example.com:80/alpha/bravo?charlie=delta#echo");

	// no user:
	check_ok("http://example.com:80/alpha/bravo?charlie=delta#echo",
	         "http://example.com:80/alpha/bravo?charlie=delta#echo");

	// empty host:
	check_ok("http://john_doe@:80/alpha/bravo?charlie=delta#echo",
	         "http://john_doe@:80/alpha/bravo?charlie=delta#echo");

	// no port:
	check_ok("http://john_doe@example.com/alpha/bravo?charlie=delta#echo",
	         "http://john_doe@example.com/alpha/bravo?charlie=delta#echo");

	// no path:
	check_ok("http://john_doe@example.com:80?charlie=delta#echo",
	         "http://john_doe@example.com:80?charlie=delta#echo");

	// no query:
	check_ok("http://john_doe@example.com:80/alpha/bravo#echo",
	         "http://john_doe@example.com:80/alpha/bravo#echo");

	// no fragment:
	check_ok("http://john_doe@example.com:80/alpha/bravo?charlie=delta",
	         "http://john_doe@example.com:80/alpha/bravo?charlie=delta");

	// user and host only:
	check_ok("john_doe@example.com",
	         "john_doe@example.com");

	// path only:
	check_ok("/alpha/bravo",
	         "/alpha/bravo");
#endif
	return 1;
}

