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
	check_consumer \
	check_headers_consumer \
	check_request_line_consumer \
	check_request_1x_consumer \
	check_chunk_line_consumer

check_PROGRAMS = \
	check_chunk_line_consumer \
	check_consumer \
	check_find_newline \
	check_is_method_valid \
	check_is_header_name_valid \
	check_is_header_value_valid \
	check_header_map_compare \
	check_header_name_less \
	check_headers_consumer \
	check_request_1x_consumer \
	check_request_line_consumer \
	check_rtrim

check_chunk_line_consumer_SOURCES = check_chunk_line_consumer.cpp
check_chunk_line_consumer_LDADD = ../libhttp.la

check_consumer_SOURCES = check_consumer.cpp
check_consumer_LDADD = ../libhttp.la

check_find_newline_SOURCES = check_find_newline.cpp
check_find_newline_LDADD = ../libhttp.la

check_header_map_compare_SOURCES = check_header_map_compare.cpp
check_header_map_compare_LDADD = ../libhttp.la

check_header_name_less_SOURCES = check_header_name_less.cpp
check_header_name_less_LDADD = ../libhttp.la

check_headers_consumer_SOURCES = check_headers_consumer.cpp
check_headers_consumer_LDADD = ../libhttp.la

check_is_method_valid_SOURCES = check_is_method_valid.cpp
check_is_method_valid_LDADD = ../libhttp.la

check_is_header_name_valid_SOURCES = check_is_header_name_valid.cpp
check_is_header_name_valid_LDADD = ../libhttp.la

check_is_header_value_valid_SOURCES = check_is_header_value_valid.cpp
check_is_header_value_valid_LDADD = ../libhttp.la

check_request_1x_consumer_SOURCES = check_request_1x_consumer.cpp
check_request_1x_consumer_LDADD = ../libhttp.la

check_request_line_consumer_SOURCES = check_request_line_consumer.cpp
check_request_line_consumer_LDADD = ../libhttp.la

check_rtrim_SOURCES = check_rtrim.cpp
check_rtrim_LDADD = ../libhttp.la

