// vim: set noet:

#include "parse.h"

#define check_parse_ok(exp_method, exp_uri, exp_major_ver, exp_minor_ver, len_limit, cons_str, rem_str) \
	do { \
		clane::http::request_line_parser parser; \
		clane::check_parse_ok_(__FILE__, __LINE__, parser, len_limit, cons_str, rem_str, [&]() { \
				check_eq(exp_method, parser.method); \
				check_eq(exp_uri, parser.uri); \
				check_eq(exp_major_ver, parser.major_ver); \
				check_eq(exp_minor_ver, parser.minor_ver); \
				}); \
	} while (false)

#define check_parse_nok(exp_stat_code, len_limit, cons_str, rem_str) \
	do { \
		clane::http::request_line_parser parser; \
		clane::check_parse_nok_(__FILE__, __LINE__, parser, len_limit, cons_str, rem_str, [&]() { \
				check_eq(exp_stat_code, parser.stat_code); \
				}); \
	} while (false)

int main() {

	// okay: canonical case
	check_parse_ok("GET", "/", 1, 1, 0, "GET / HTTP/1.1\r\n", "\r\n");
	check_parse_ok("GET", "/", 1, 1, 32, "GET / HTTP/1.1\r\n", "\r\n");

	// okay: simple newline
	check_parse_ok("GET", "/", 1, 1, 0, "GET / HTTP/1.1\n", "\n");
	check_parse_ok("GET", "/", 1, 1, 32, "GET / HTTP/1.1\n", "\n");

	// okay: different version values
	check_parse_ok("GET", "/", 0, 9, 0, "GET / HTTP/0.9\n", "\n");
	check_parse_ok("GET", "/", 1, 0, 0, "GET / HTTP/1.0\n", "\n");
	check_parse_ok("GET", "/", 2, 0, 0, "GET / HTTP/2.0\n", "\n");

	// okay: complex URI reference
	check_parse_ok("GET", "/alpha/bravo", 1, 1, 0, "GET /alpha/bravo HTTP/1.1\r\n", "\r\n");
	check_parse_ok("GET", "/alpha/bravo?charlie=delta", 1, 1, 0, "GET /alpha/bravo?charlie=delta HTTP/1.1\r\n", "\r\n");
	check_parse_ok("GET", "http://foobar/alpha/bravo?charlie=delta", 1, 1, 0,
			"GET http://foobar/alpha/bravo?charlie=delta HTTP/1.1\r\n", "\r\n");

	// error: empty line
	check_parse_nok(clane::http::status_code::bad_request, 0, "", "\r\n");

	// error: line too long
	check_parse_nok(clane::http::status_code::bad_request, 1,  "GE", "T /alpha HTTP/1.1\r\n\r\n");
	check_parse_nok(clane::http::status_code::request_uri_too_long, 6, "GET /al", "pha HTTP/1.1\r\n\r\n");
	check_parse_nok(clane::http::status_code::request_uri_too_long, 12, "GET /alpha HT", "TP/1.1\r\n\r\n");
	check_parse_nok(clane::http::status_code::request_uri_too_long, 19, "GET /alpha HTTP/1.1\r", "\n\r\n");
	check_parse_nok(clane::http::status_code::request_uri_too_long, 20, "GET /alpha HTTP/1.1\r\n", "\r\n");

	// error: missing version field
	check_parse_nok(clane::http::status_code::bad_request, 0, "GET /\r", "\n\r\n");
	check_parse_nok(clane::http::status_code::bad_request, 0, "GET /\n", "\n");

	// error: empty method field
	check_parse_nok(clane::http::status_code::bad_request, 0, " ", "/ HTTP/1.1\r\n");

	// error: spurious carriage returns
	check_parse_nok(clane::http::status_code::bad_request, 0, "\r", "GET /alpha HTTP/1.1\r\n");
	check_parse_nok(clane::http::status_code::bad_request, 0, "GE\r", "T /alpha HTTP/1.1\r\n");
	check_parse_nok(clane::http::status_code::bad_request, 0, "GET\r", " /alpha HTTP/1.1\r\n");
	//check_parse_nok(clane::http::status_code::bad_request, 0, "GET \r", "/alpha HTTP/1.1\r\n");
	//check_parse_nok(clane::http::status_code::bad_request, 0, "GET /al\r", "pha HTTP/1.1\r\n");
	//check_parse_nok(clane::http::status_code::bad_request, 0, "GET /alpha\r", "HTTP/1.1\r\n");
	check_parse_nok(clane::http::status_code::bad_request, 0, "GET /alpha \r", "HTTP/1.1\r\n");
	check_parse_nok(clane::http::status_code::bad_request, 0, "GET /alpha HT\r", "TP/1.1\r\n");
	check_parse_nok(clane::http::status_code::bad_request, 0, "GET /alpha HTTP/1.1\r\r", "\n");

	// error: syntactically invalid method
	check_parse_nok(clane::http::status_code::bad_request, 0, "GET() ", "/ HTTP/1.1\r\n");

	// error: syntactically invalid URI reference
	check_parse_nok(clane::http::status_code::bad_request, 0, "GET 1234-bad-scheme://alpha/bravo ", "HTTP/1.1\r\n");

	return 0;
}

