AM_CXXFLAGS = -Wall -Werror -std=c++11 -I../../check
libcheck = ../../check/libcheck.la
libhttp_deps = ../../uri/liburi.la

leak-check: check
	../../leak_check $(TESTS)

TESTS = \
	check_find_newline \
	check_rtrim \
	check_header_name_less \
	check_header_map_compare \
	check_is_method_valid \
	check_is_header_name_valid \
	check_is_header_value_valid \
	check_parse_request_line \
	check_parse_headers

check_LTLIBRARIES = libparse.la

check_PROGRAMS = \
	check_find_newline \
	check_header_map_compare \
	check_header_name_less \
	check_is_header_name_valid \
	check_is_header_value_valid \
	check_is_method_valid \
	check_parse_headers \
	check_parse_request_line \
	check_rtrim

check_find_newline_SOURCES = check_find_newline.cpp
check_find_newline_LDADD = $(libcheck) $(libhttp_deps)

check_header_map_compare_SOURCES = check_header_map_compare.cpp
check_header_map_compare_LDADD = $(libcheck) ../libhttp.la

check_header_name_less_SOURCES = check_header_name_less.cpp
check_header_name_less_LDADD = $(libcheck) ../libhttp.la

check_is_header_name_valid_SOURCES = check_is_header_name_valid.cpp
check_is_header_name_valid_LDADD = $(libcheck) $(libhttp_deps)

check_is_header_value_valid_SOURCES = check_is_header_value_valid.cpp
check_is_header_value_valid_LDADD = $(libcheck) $(libhttp_deps)

check_is_method_valid_SOURCES = check_is_method_valid.cpp
check_is_method_valid_LDADD = $(libcheck) $(libhttp_deps)

check_parse_headers_SOURCES = check_parse_headers.cpp
check_parse_headers_LDADD = libparse.la

check_parse_request_line_SOURCES = check_parse_request_line.cpp
check_parse_request_line_LDADD = libparse.la

check_rtrim_SOURCES = check_rtrim.cpp
check_rtrim_LDADD = $(libcheck) $(libhttp_deps)

libparse_la_SOURCES = parse.cpp parse.h
libparse_la_LIBADD = $(libcheck) ../libhttp.la
