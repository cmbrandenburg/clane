AM_CXXFLAGS = -Wall -Werror -std=c++11 -I../../check

TESTS = \
	check_posix_errno_to_string \
	check_posix_fd \
	check_http_server_run_term

check_PROGRAMS = \
	check_http_server_run_term \
	check_posix_errno_to_string \
	check_posix_fd

check_http_server_run_term_SOURCES = check_http_server_run_term.cpp
check_http_server_run_term_LDFLAGS = ../libclane.la

check_posix_errno_to_string_SOURCES = check_posix_errno_to_string.cpp
check_posix_errno_to_string_LDFLAGS = ../libclane.la

check_posix_fd_SOURCES = check_posix_fd.cpp
check_posix_fd_LDFLAGS = ../libclane.la

