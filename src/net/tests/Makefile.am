AM_CXXFLAGS = -Wall -Werror -std=c++11

TESTS = \
	check_posix_errno_to_string \
	check_posix_fd \
	check_event \
	check_tcp_connect_accept

check_PROGRAMS = \
	check_event \
	check_posix_errno_to_string \
	check_posix_fd \
	check_tcp_connect_accept

check_event_SOURCES = check_event.cpp
check_event_LDFLAGS = ../libnet.la

check_posix_errno_to_string_SOURCES = check_posix_errno_to_string.cpp
check_posix_errno_to_string_LDFLAGS = ../libnet.la

check_posix_fd_SOURCES = check_posix_fd.cpp
check_posix_fd_LDFLAGS = ../libnet.la

check_tcp_connect_accept_SOURCES = check_tcp_connect_accept.cpp
check_tcp_connect_accept_LDFLAGS = ../libnet.la

