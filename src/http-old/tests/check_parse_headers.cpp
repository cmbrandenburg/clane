// vim: set noet:

#include "parse.h"
#include <initializer_list>

#define check_ok(len_limit, cons_str, rem_str, ...) \
	do { \
		check_ok_(__FILE__, __LINE__, len_limit, cons_str, rem_str, ## __VA_ARGS__); \
	} while (false)

#define check_nok(len_limit, cons_str, rem_str, exp_stat_code) \
	do { \
		check_nok_(__FILE__, __LINE__, len_limit, cons_str, rem_str, exp_stat_code); \
	} while (false)

static void check_nok_(char const *sname, int sline, size_t len_limit, char const *cons_str, char const *rem_str,
		clane::http::status_code exp_stat_code);
static void check_ok_(char const *sname, int sline, size_t len_limit, char const *cons_str, char const *rem_str,
		std::initializer_list<std::string> const &exp_hdrs);

void check_nok_(char const *sname, int sline, size_t len_limit, char const *cons_str, char const *rem_str,
		clane::http::status_code exp_stat_code) {
	clane::http::headers_parser parser;
	clane::check_parse_nok_(sname, sline, parser, len_limit, cons_str, rem_str, [&]() {
		check_eq(exp_stat_code, parser.stat_code);
	});
}

void check_ok_(char const *sname, int sline, size_t len_limit, char const *cons_str, char const *rem_str,
		std::initializer_list<std::string> const &exp_hdrs) {

	// construct expected header map:
	clane::http::header_map exp_map;
	for (auto i = exp_hdrs.begin(); i != exp_hdrs.end(); ++i) {
		std::string const &name = *i;
		++i;
		if (i == exp_hdrs.end())
			abort();
		std::string const &value = *i;
		exp_map.insert(clane::http::header_map::value_type(name, value));
	}

	// check:
	clane::http::headers_parser parser;
	clane::check_parse_ok_(sname, sline, parser, len_limit, cons_str, rem_str, [&]() {
		check_eq(exp_map, parser.hdrs);
	});
}

int main() {

	// one header:
	check_ok(0, "alpha: bravo\r\n\r\n", "", {"alpha", "bravo"});
	check_ok(0, "alpha: bravo\n\n", "", {"alpha", "bravo"});

	// multiple headers:
	check_ok(0, "alpha: bravo\r\ncharlie: delta\r\n\r\n", "", {"alpha", "bravo", "charlie", "delta"});
	check_ok(0, "alpha: bravo\ncharlie: delta\n\n", "", {"alpha", "bravo", "charlie", "delta"});

	// zero headers:
	check_ok(0, "\r\n", "", {});
	check_ok(0, "\n", "", {});

	// linear whitespace--multi-line header value
	check_ok(0, "alpha: bravo\r\n charlie\r\n\r\n", "", {"alpha", "bravo charlie"});
	check_ok(0, "alpha: bravo\n charlie\n\n", "", {"alpha", "bravo charlie"});
	check_ok(0, "alpha: bravo\r\n\tcharlie\r\n\r\n", "", {"alpha", "bravo charlie"});
	check_ok(0, "alpha: bravo\n\tcharlie\n\n", "", {"alpha", "bravo charlie"});
	check_ok(0, "alpha: bravo\r\n charlie\r\n delta\r\n\r\n", "", {"alpha", "bravo charlie delta"});

	// non-canonical whitespace
	check_ok(0, "alpha  \t\t  : bravo\r\n\r\n", "", {"alpha", "bravo"}); 
	check_ok(0, "alpha:bravo\r\n\r\n", "", {"alpha", "bravo"}); 
	check_ok(0, "alpha:  \t\t  bravo\r\n\r\n", "", {"alpha", "bravo"}); 
	check_ok(0, "alpha: bravo  \t\t  \r\n\r\n", "", {"alpha", "bravo"}); 

	// length limits:
	check_nok(1, "al", "pha: bravo\r\n\r\n", clane::http::status_code::bad_request);
	check_nok(5, "alpha:", " bravo\r\n\r\n", clane::http::status_code::bad_request);
	check_nok(6, "alpha: ", "bravo\r\n\r\n", clane::http::status_code::bad_request);
	check_nok(11, "alpha: bravo", "\r\n\r\n", clane::http::status_code::bad_request);
	check_nok(12, "alpha: bravo\r", "\n\r\n", clane::http::status_code::bad_request);
	check_nok(13, "alpha: bravo\r\n", "\r\n", clane::http::status_code::bad_request);
	check_nok(14, "alpha: bravo\r\n\r", "\n", clane::http::status_code::bad_request);
	check_nok(14, "alpha: bravo\r\n\r", "\n", clane::http::status_code::bad_request);
	check_nok(15, "alpha: bravo\r\n\r\n", "", clane::http::status_code::bad_request);
	check_ok(16, "alpha: bravo\r\n\r\n", "blah", {"alpha", "bravo"});

	// zero-length name:
	check_nok(0, ":", " bravo\r\n\r\n", clane::http::status_code::bad_request);

	// invalid name:
	check_nok(0, "[alpha]:", " bravo\r\n\r\n", clane::http::status_code::bad_request);
	check_nok(0, "alpha\rbravo:", " charlie\rcharlie\r\n\r\n", clane::http::status_code::bad_request);

	// invalid value:
	check_nok(0, "alpha: bravo\001charlie\r\n\r\n", "", clane::http::status_code::bad_request);
	check_nok(0, "alpha: bravo\177charlie\r\n\r\n", "", clane::http::status_code::bad_request);
	check_nok(0, "alpha: bravo\rc", "harlie\r\n\r\n", clane::http::status_code::bad_request);

	return 0;
}

