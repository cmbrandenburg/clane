AM_CXXFLAGS = -Wall -Werror -std=c++11 -I../../check

leak-check: check
	../../leak_check $(TESTS)

TESTS = \
	check_header_name_less \
	check_header_map_compare

check_PROGRAMS = \
	check_header_map_compare \
	check_header_name_less

check_header_map_compare_SOURCES = check_header_map_compare.cpp
check_header_map_compare_LDADD = ../libhttp.la

check_header_name_less_SOURCES = check_header_name_less.cpp
check_header_name_less_LDADD = ../libhttp.la

