// vim: set noet:

#include "../uri.h"
#include "../../check/check.h"

int main() {

	clane::uri::uri u;

	// empty:
	check_true(clane::uri::parse_uri_reference(u, ""));
	check_eq("", u.scheme);
	check_eq("", u.user_info);
	check_eq("", u.host);
	check_eq("", u.port);
	check_eq("", u.path);
	check_eq("", u.query);
	check_eq("", u.fragment);

	// all fields:
	check_true(clane::uri::parse_uri_reference(u, "http://alpha@bravo:1234/charlie/delta?echo=foxtrot#golf"));
	check_eq("http", u.scheme);
	check_eq("alpha", u.user_info);
	check_eq("bravo", u.host);
	check_eq("1234", u.port);
	check_eq("/charlie/delta", u.path);
	check_eq("echo=foxtrot", u.query);
	check_eq("golf", u.fragment);

	// percent-encoded:
	check_true(clane::uri::parse_uri_reference(u, "http://%5Balpha%5D@%5Bbravo%5D/charlie%3Fdelta?echo=%5Bfoxtrot%5D#%5Bgolf%5D"));
	check_eq("http", u.scheme);
	check_eq("[alpha]", u.user_info);
	check_eq("[bravo]", u.host);
	check_eq("", u.port);
	check_eq("/charlie?delta", u.path);
	check_eq("echo=[foxtrot]", u.query);
	check_eq("[golf]", u.fragment);

	// percent-encoded, lowercase:
	check_true(clane::uri::parse_uri_reference(u, "http://%5balpha%5d@%5bbravo%5d/charlie%3Fdelta?echo=%5bfoxtrot%5d#%5bgolf%5d"));
	check_eq("http", u.scheme);
	check_eq("[alpha]", u.user_info);
	check_eq("[bravo]", u.host);
	check_eq("", u.port);
	check_eq("/charlie?delta", u.path);
	check_eq("echo=[foxtrot]", u.query);
	check_eq("[golf]", u.fragment);

	// scheme only:
	check_true(clane::uri::parse_uri_reference(u, "http:"));
	check_eq("http", u.scheme);
	check_eq("", u.user_info);
	check_eq("", u.host);
	check_eq("", u.port);
	check_eq("", u.path);
	check_eq("", u.query);
	check_eq("", u.fragment);

	// user info only:
	check_true(clane::uri::parse_uri_reference(u, "//alpha@"));
	check_eq("", u.scheme);
	check_eq("alpha", u.user_info);
	check_eq("", u.host);
	check_eq("", u.port);
	check_eq("", u.path);
	check_eq("", u.query);
	check_eq("", u.fragment);

	// host only:
	check_true(clane::uri::parse_uri_reference(u, "//alpha"));
	check_eq("", u.scheme);
	check_eq("", u.user_info);
	check_eq("alpha", u.host);
	check_eq("", u.port);
	check_eq("", u.path);
	check_eq("", u.query);
	check_eq("", u.fragment);

	// port only:
	check_true(clane::uri::parse_uri_reference(u, "//:1234"));
	check_eq("", u.scheme);
	check_eq("", u.user_info);
	check_eq("", u.host);
	check_eq("1234", u.port);
	check_eq("", u.path);
	check_eq("", u.query);
	check_eq("", u.fragment);

	// path only:
	check_true(clane::uri::parse_uri_reference(u, "/alpha/bravo"));
	check_eq("", u.scheme);
	check_eq("", u.user_info);
	check_eq("", u.host);
	check_eq("", u.port);
	check_eq("/alpha/bravo", u.path);
	check_eq("", u.query);
	check_eq("", u.fragment);

	// query only:
	check_true(clane::uri::parse_uri_reference(u, "?alpha"));
	check_eq("", u.scheme);
	check_eq("", u.user_info);
	check_eq("", u.host);
	check_eq("", u.port);
	check_eq("", u.path);
	check_eq("alpha", u.query);
	check_eq("", u.fragment);

	// fragment only:
	check_true(clane::uri::parse_uri_reference(u, "#alpha"));
	check_eq("", u.scheme);
	check_eq("", u.user_info);
	check_eq("", u.host);
	check_eq("", u.port);
	check_eq("", u.path);
	check_eq("", u.query);
	check_eq("alpha", u.fragment);

	// relative path, no scheme:
	check_true(clane::uri::parse_uri_reference(u, "alpha/bravo"));
	check_eq("", u.scheme);
	check_eq("", u.user_info);
	check_eq("", u.host);
	check_eq("", u.port);
	check_eq("alpha/bravo", u.path);
	check_eq("", u.query);
	check_eq("", u.fragment);

	// relative path, with scheme:
	check_true(clane::uri::parse_uri_reference(u, "foo:alpha/bravo"));
	check_eq("foo", u.scheme);
	check_eq("", u.user_info);
	check_eq("", u.host);
	check_eq("", u.port);
	check_eq("alpha/bravo", u.path);
	check_eq("", u.query);
	check_eq("", u.fragment);

	// path with colon in non-first component:
	check_true(clane::uri::parse_uri_reference(u, "alpha/bravo:charlie"));
	check_eq("", u.scheme);
	check_eq("", u.user_info);
	check_eq("", u.host);
	check_eq("", u.port);
	check_eq("alpha/bravo:charlie", u.path);
	check_eq("", u.query);
	check_eq("", u.fragment);

	// invalidity causes uri argument to be unmodified:
	u.scheme = "http";
	u.user_info = "alpha";
	u.host = "bravo";
	u.port = "1234";
	u.path = "/charlie";
	u.query = "delta";
	u.fragment = "echo";
	{
		clane::uri::uri cp(u);
		check_false(clane::uri::parse_uri_reference(u, "1234-bad-scheme:"));
		check_eq(u, cp);
	}

	// invalid scheme:
	check_false(clane::uri::parse_uri_reference(u, "1234:"));

	// invalid user info:
	check_false(clane::uri::parse_uri_reference(u, "http://[bad-user]@"));

	// invalid host:
	check_false(clane::uri::parse_uri_reference(u, "http://[bad-host]"));

	// invalid port:
	check_false(clane::uri::parse_uri_reference(u, "http://:bad-port"));

	// invalid path:
	check_false(clane::uri::parse_uri_reference(u, "http://host/[bad-path]"));

	// invalid query:
	check_false(clane::uri::parse_uri_reference(u, "http://host/?[bad-query]"));

	// invalid fragment:
	check_false(clane::uri::parse_uri_reference(u, "http://host/#[bad-fragment]"));

	// short, absolute path:
	check_true(clane::uri::parse_uri_reference(u, "/"));
	check_eq("", u.scheme);
	check_eq("", u.user_info);
	check_eq("", u.host);
	check_eq("", u.port);
	check_eq("/", u.path);
	check_eq("", u.query);
	check_eq("", u.fragment);


	return 0;
}

