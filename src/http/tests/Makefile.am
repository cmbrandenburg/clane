AM_CXXFLAGS = -Wall -Werror -std=c++11

TESTS = \
	check_header_name_less \
	check_server_run_term

check_PROGRAMS = \
	check_header_name_less \
	check_server_run_term

check_header_name_less_SOURCES = check_header_name_less.cpp
check_header_name_less_LDFLAGS = ../libhttp.la

check_server_run_term_SOURCES = check_server_run_term.cpp
check_server_run_term_LDFLAGS = ../libhttp.la

