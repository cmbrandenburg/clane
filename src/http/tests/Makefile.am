AM_CXXFLAGS = -Wall -Werror -std=c++11 -I../../check

leak-check: check
	../../leak_check $(TESTS)

TESTS = \
	check_header_name_less \
	check_header_map_compare \
	check_find_newline \
	check_rtrim \
	check_is_method_valid \
	check_is_header_name_valid \
	check_is_header_value_valid \
	check_parser \
	check_headers_parser \
	check_request_line_parser

check_PROGRAMS = \
	check_find_newline \
	check_is_method_valid \
	check_is_header_name_valid \
	check_is_header_value_valid \
	check_header_map_compare \
	check_header_name_less \
	check_headers_parser \
	check_parser \
	check_request_line_parser \
	check_rtrim

check_find_newline_SOURCES = check_find_newline.cpp
check_find_newline_LDADD = ../libhttp.la

check_header_map_compare_SOURCES = check_header_map_compare.cpp
check_header_map_compare_LDADD = ../libhttp.la

check_header_name_less_SOURCES = check_header_name_less.cpp
check_header_name_less_LDADD = ../libhttp.la

check_headers_parser_SOURCES = check_headers_parser.cpp
check_headers_parser_LDADD = ../libhttp.la

check_is_method_valid_SOURCES = check_is_method_valid.cpp
check_is_method_valid_LDADD = ../libhttp.la

check_is_header_name_valid_SOURCES = check_is_header_name_valid.cpp
check_is_header_name_valid_LDADD = ../libhttp.la

check_is_header_value_valid_SOURCES = check_is_header_value_valid.cpp
check_is_header_value_valid_LDADD = ../libhttp.la

check_parser_SOURCES = check_parser.cpp
check_parser_LDADD = ../libhttp.la

check_request_line_parser_SOURCES = check_request_line_parser.cpp
check_request_line_parser_LDADD = ../libhttp.la

check_rtrim_SOURCES = check_rtrim.cpp
check_rtrim_LDADD = ../libhttp.la

