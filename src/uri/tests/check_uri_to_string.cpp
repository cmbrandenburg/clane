// vim: set noet:

#include "../uri.h"
#include "../../check/check.h"

int main() {
	clane::uri::uri u;

	// empty:
	check_eq("", static_cast<std::string>(u));

	// all fields:
	u.clear();
	u.scheme = "http";
	u.user_info = "alpha";
	u.host = "bravo";
	u.port = "1234";
	u.path = "/charlie";
	u.query = "delta=echo";
	u.fragment = "foxtrot";
	check_eq("http://alpha@bravo:1234/charlie?delta=echo#foxtrot", static_cast<std::string>(u));

	// all fields, with percent-encoding:
	u.clear();
	u.scheme = "http";
	u.user_info = "/alpha/";
	u.host = "#bravo#";
	u.path = "/charlie?delta/echo";
	u.query = "foxtrot=[golf]";
	u.fragment = "[hotel]";
	check_eq("http://%2Falpha%2F@%23bravo%23/charlie%3Fdelta/echo?foxtrot=%5Bgolf%5D#%5Bhotel%5D", static_cast<std::string>(u));

	// authority without scheme:
	u.clear();
	u.host = "alpha";
	u.path = "/bravo";
	check_eq("//alpha/bravo", static_cast<std::string>(u));

	// path only:
	u.clear();
	u.path = "/";
	check_eq("/", static_cast<std::string>(u));

	// IP literal:
	u.clear();
	u.scheme = "http";
	u.host = "[::1]";
	u.path = "/";
	check_eq("http://[::1]/", static_cast<std::string>(u));

	return 0;
}

