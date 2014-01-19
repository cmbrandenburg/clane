// vim: set noet:

#include "../../check/check.h"
#include "../uri.hpp"

int main() {

	clane::uri::uri u;

	// empty:
	check(clane::uri::parse_uri_reference(u, ""));
	check("" == u.scheme);
	check("" == u.user_info);
	check("" == u.host);
	check("" == u.port);
	check("" == u.path);
	check("" == u.query);
	check("" == u.fragment);

	// all fields:
	check(clane::uri::parse_uri_reference(u, "http://alpha@bravo:1234/charlie/delta?echo=foxtrot#golf"));
	check("http" == u.scheme);
	check("alpha" == u.user_info);
	check("bravo" == u.host);
	check("1234" == u.port);
	check("/charlie/delta" == u.path);
	check("echo=foxtrot" == u.query);
	check("golf" == u.fragment);

	// percent-encoded:
	check(clane::uri::parse_uri_reference(u, "http://%5Balpha%5D@%5Bbravo%5D/charlie%3Fdelta?echo=%5Bfoxtrot%5D#%5Bgolf%5D"));
	check("http" == u.scheme);
	check("[alpha]" == u.user_info);
	check("[bravo]" == u.host);
	check("" == u.port);
	check("/charlie?delta" == u.path);
	check("echo=[foxtrot]" == u.query);
	check("[golf]" == u.fragment);

	// percent-encoded, lowercase:
	check(clane::uri::parse_uri_reference(u, "http://%5balpha%5d@%5bbravo%5d/charlie%3Fdelta?echo=%5bfoxtrot%5d#%5bgolf%5d"));
	check("http" == u.scheme);
	check("[alpha]" == u.user_info);
	check("[bravo]" == u.host);
	check("" == u.port);
	check("/charlie?delta" == u.path);
	check("echo=[foxtrot]" == u.query);
	check("[golf]" == u.fragment);

	// scheme only:
	check(clane::uri::parse_uri_reference(u, "http:"));
	check("http" == u.scheme);
	check("" == u.user_info);
	check("" == u.host);
	check("" == u.port);
	check("" == u.path);
	check("" == u.query);
	check("" == u.fragment);

	// user info only:
	check(clane::uri::parse_uri_reference(u, "//alpha@"));
	check("" == u.scheme);
	check("alpha" == u.user_info);
	check("" == u.host);
	check("" == u.port);
	check("" == u.path);
	check("" == u.query);
	check("" == u.fragment);

	// host only:
	check(clane::uri::parse_uri_reference(u, "//alpha"));
	check("" == u.scheme);
	check("" == u.user_info);
	check("alpha" == u.host);
	check("" == u.port);
	check("" == u.path);
	check("" == u.query);
	check("" == u.fragment);

	// port only:
	check(clane::uri::parse_uri_reference(u, "//:1234"));
	check("" == u.scheme);
	check("" == u.user_info);
	check("" == u.host);
	check("1234" == u.port);
	check("" == u.path);
	check("" == u.query);
	check("" == u.fragment);

	// path only:
	check(clane::uri::parse_uri_reference(u, "/alpha/bravo"));
	check("" == u.scheme);
	check("" == u.user_info);
	check("" == u.host);
	check("" == u.port);
	check("/alpha/bravo" == u.path);
	check("" == u.query);
	check("" == u.fragment);

	// query only:
	check(clane::uri::parse_uri_reference(u, "?alpha"));
	check("" == u.scheme);
	check("" == u.user_info);
	check("" == u.host);
	check("" == u.port);
	check("" == u.path);
	check("alpha" == u.query);
	check("" == u.fragment);

	// fragment only:
	check(clane::uri::parse_uri_reference(u, "#alpha"));
	check("" == u.scheme);
	check("" == u.user_info);
	check("" == u.host);
	check("" == u.port);
	check("" == u.path);
	check("" == u.query);
	check("alpha" == u.fragment);

	// relative path, no scheme:
	check(clane::uri::parse_uri_reference(u, "alpha/bravo"));
	check("" == u.scheme);
	check("" == u.user_info);
	check("" == u.host);
	check("" == u.port);
	check("alpha/bravo" == u.path);
	check("" == u.query);
	check("" == u.fragment);

	// relative path, with scheme:
	check(clane::uri::parse_uri_reference(u, "foo:alpha/bravo"));
	check("foo" == u.scheme);
	check("" == u.user_info);
	check("" == u.host);
	check("" == u.port);
	check("alpha/bravo" == u.path);
	check("" == u.query);
	check("" == u.fragment);

	// path with colon in non-first component:
	check(clane::uri::parse_uri_reference(u, "alpha/bravo:charlie"));
	check("" == u.scheme);
	check("" == u.user_info);
	check("" == u.host);
	check("" == u.port);
	check("alpha/bravo:charlie" == u.path);
	check("" == u.query);
	check("" == u.fragment);

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
		check(!clane::uri::parse_uri_reference(u, "1234-bad-scheme:"));
		check(u == cp);
	}

	// invalid scheme:
	check(!clane::uri::parse_uri_reference(u, "1234:"));

	// invalid user info:
	check(!clane::uri::parse_uri_reference(u, "http://[bad-user]@"));

	// invalid host:
	check(!clane::uri::parse_uri_reference(u, "http://[bad-host]"));

	// invalid port:
	check(!clane::uri::parse_uri_reference(u, "http://:bad-port"));

	// invalid path:
	check(!clane::uri::parse_uri_reference(u, "http://host/[bad-path]"));

	// invalid query:
	check(!clane::uri::parse_uri_reference(u, "http://host/?[bad-query]"));

	// invalid fragment:
	check(!clane::uri::parse_uri_reference(u, "http://host/#[bad-fragment]"));

	// short, absolute path:
	check(clane::uri::parse_uri_reference(u, "/"));
	check("" == u.scheme);
	check("" == u.user_info);
	check("" == u.host);
	check("" == u.port);
	check("/" == u.path);
	check("" == u.query);
	check("" == u.fragment);


	return 0;
}

