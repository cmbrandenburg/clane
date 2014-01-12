// vim: set noet:

#include "../../check/check.h"
#include "../uri.hpp"

int main() {
	clane::uri::uri u;

	// empty:
	check("" == u.to_string());

	// all fields:
	u.clear();
	u.scheme = "http";
	u.user_info = "alpha";
	u.host = "bravo";
	u.port = "1234";
	u.path = "/charlie";
	u.query = "delta=echo";
	u.fragment = "foxtrot";
	check("http://alpha@bravo:1234/charlie?delta=echo#foxtrot" == u.to_string());

	// all fields, with percent-encoding:
	u.clear();
	u.scheme = "http";
	u.user_info = "/alpha/";
	u.host = "#bravo#";
	u.path = "/charlie?delta/echo";
	u.query = "foxtrot=[golf]";
	u.fragment = "[hotel]";
	check("http://%2Falpha%2F@%23bravo%23/charlie%3Fdelta/echo?foxtrot=%5Bgolf%5D#%5Bhotel%5D" == u.to_string());

	// authority without scheme:
	u.clear();
	u.host = "alpha";
	u.path = "/bravo";
	check("//alpha/bravo" == u.to_string());

	// path only:
	u.clear();
	u.path = "/";
	check("/" == u.to_string());

	// IP literal:
	u.clear();
	u.scheme = "http";
	u.host = "[::1]";
	u.path = "/";
	check("http://[::1]/" == u.to_string());

	return 0;
}

