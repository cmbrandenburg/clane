// vim: set noet:

#include "clane_check.hpp"
#include "../clane_uri.hpp"

using namespace clane;

#define check_ok(in, exp_scheme, exp_user, exp_host, exp_port, exp_path, exp_query, exp_fragment) \
	do { \
		std::error_code e; \
		uri::uri u = uri::parse_uri_reference(in, e); \
		check(!e); \
		check(u.scheme == exp_scheme); \
		check(u.user == exp_user); \
		check(u.host == exp_host); \
		check(u.port == exp_port); \
		check(u.path == exp_path); \
		check(u.query == exp_query); \
		check(u.fragment == exp_fragment); \
	} while (false)

#define check_nok(in, exp_e) \
	do { \
		std::error_code e; \
		uri::uri u = uri::parse_uri_reference(in, e); \
		check(u.empty()); \
		check(e.category().name() == std::string("URI")); \
		check(e.value() == static_cast<int>(exp_e)); \
	} while (false)

int main() {

	// empty:
	check_ok("",
			/* scheme   */ "",
			/* user     */ "",
			/* host     */ "",
			/* port     */ "",
			/* path     */ "",
			/* query    */ "",
			/* fragment */ "");

	// all fields:
	check_ok("http://alpha@bravo:1234/charlie/delta?echo=foxtrot#golf",
			/* scheme   */ "http",
			/* user     */ "alpha",
			/* host     */ "bravo",
			/* port     */ "1234",
			/* path     */ "/charlie/delta",
			/* query    */ "echo=foxtrot",
			/* fragment */ "golf");

	// percent-encoded:
	check_ok("http://%5Balpha%5D@%5Bbravo%5D/charlie%3Fdelta?echo=%5Bfoxtrot%5D#%5Bgolf%5D",
			/* scheme   */ "http",
			/* user     */ "[alpha]",
			/* host     */ "[bravo]",
			/* port     */ "",
			/* path     */ "/charlie?delta",
			/* query    */ "echo=[foxtrot]",
			/* fragment */ "[golf]");

	// percent-encoded, lowercase:
	check_ok("http://%5balpha%5d@%5Bbravo%5d/charlie%3fdelta?echo=%5bfoxtrot%5d#%5bgolf%5d",
			/* scheme   */ "http",
			/* user     */ "[alpha]",
			/* host     */ "[bravo]",
			/* port     */ "",
			/* path     */ "/charlie?delta",
			/* query    */ "echo=[foxtrot]",
			/* fragment */ "[golf]");

	// IPv4 address:
	check_ok("http://alpha@127.0.0.1:1234/charlie/delta?echo=foxtrot#golf",
			/* scheme   */ "http",
			/* user     */ "alpha",
			/* host     */ "127.0.0.1",
			/* port     */ "1234",
			/* path     */ "/charlie/delta",
			/* query    */ "echo=foxtrot",
			/* fragment */ "golf");

	// IPv6 address:
	check_ok("http://alpha@[::1]:1234/charlie/delta?echo=foxtrot#golf",
			/* scheme   */ "http",
			/* user     */ "alpha",
			/* host     */ "[::1]",
			/* port     */ "1234",
			/* path     */ "/charlie/delta",
			/* query    */ "echo=foxtrot",
			/* fragment */ "golf");

	// IPvFuture address:
	check_ok("http://alpha@[v7.blahblahblah]:1234/charlie/delta?echo=foxtrot#golf",
			/* scheme   */ "http",
			/* user     */ "alpha",
			/* host     */ "[v7.blahblahblah]",
			/* port     */ "1234",
			/* path     */ "/charlie/delta",
			/* query    */ "echo=foxtrot",
			/* fragment */ "golf");

	// scheme only:
	check_ok("http:",
			/* scheme   */ "http",
			/* user     */ "",
			/* host     */ "",
			/* port     */ "",
			/* path     */ "",
			/* query    */ "",
			/* fragment */ "");

	// user only:
	check_ok("//alpha@",
			/* scheme   */ "",
			/* user     */ "alpha",
			/* host     */ "",
			/* port     */ "",
			/* path     */ "",
			/* query    */ "",
			/* fragment */ "");

	// host only:
	check_ok("//alpha",
			/* scheme   */ "",
			/* user     */ "",
			/* host     */ "alpha",
			/* port     */ "",
			/* path     */ "",
			/* query    */ "",
			/* fragment */ "");

	// port only:
	check_ok("//:1234",
			/* scheme   */ "",
			/* user     */ "",
			/* host     */ "",
			/* port     */ "1234",
			/* path     */ "",
			/* query    */ "",
			/* fragment */ "");

	// path only:
	check_ok("/alpha/bravo",
			/* scheme   */ "",
			/* user     */ "",
			/* host     */ "",
			/* port     */ "",
			/* path     */ "/alpha/bravo",
			/* query    */ "",
			/* fragment */ "");

	// query only:
	check_ok("?alpha",
			/* scheme   */ "",
			/* user     */ "",
			/* host     */ "",
			/* port     */ "",
			/* path     */ "",
			/* query    */ "alpha",
			/* fragment */ "");

	// fragment only:
	check_ok("#alpha",
			/* scheme   */ "",
			/* user     */ "",
			/* host     */ "",
			/* port     */ "",
			/* path     */ "",
			/* query    */ "",
			/* fragment */ "alpha");

	// relative path, no scheme:
	check_ok("alpha/bravo",
			/* scheme   */ "",
			/* user     */ "",
			/* host     */ "",
			/* port     */ "",
			/* path     */ "alpha/bravo",
			/* query    */ "",
			/* fragment */ "");

	// relative path, with scheme:
	check_ok("mailto:john_doe@example.com",
			/* scheme   */ "mailto",
			/* user     */ "",
			/* host     */ "",
			/* port     */ "",
			/* path     */ "john_doe@example.com",
			/* query    */ "",
			/* fragment */ "");

	// path with colon in non-first component:
	check_ok("alpha/bravo:charlie",
			/* scheme   */ "",
			/* user     */ "",
			/* host     */ "",
			/* port     */ "",
			/* path     */ "alpha/bravo:charlie",
			/* query    */ "",
			/* fragment */ "");

	// short, absolute path:
	check_ok("/",
			/* scheme   */ "",
			/* user     */ "",
			/* host     */ "",
			/* port     */ "",
			/* path     */ "/",
			/* query    */ "",
			/* fragment */ "");

	// invalid scheme:
	check_nok("1234:", uri::error_code::invalid_scheme);

	// invalid user:
	check_nok("http:// bad-user @", uri::error_code::invalid_user);

	// invalid host:
	check_nok("http:// bad-host ", uri::error_code::invalid_host);

	// invalid port:
	check_nok("http://:bad-port", uri::error_code::invalid_port);

	// invalid path:
	check_nok("http://host/[bad-path]", uri::error_code::invalid_path);

	// invalid query:
	check_nok("http://host/?[bad-query]", uri::error_code::invalid_query);

	// invalid fragment:
	check_nok("http://host/#[bad-fragment]", uri::error_code::invalid_fragment);

}

