AM_CXXFLAGS = -Wall -Werror -std=c++11 -I../../check
libcheck = ../../check/libcheck.la

leak-check: check
	../../leak_check $(TESTS)

TESTS = \
	check_errno_to_string \
	check_fd \
	check_split_ipvx_host_port \
	check_tcp_listen_and_accept \
	check_tcp_accept_nonblocking \
	check_tcp_send_and_recv \
	check_tcp_recv_shutdown \
	check_smux_empty \
	check_mmux_empty \
	check_smux_one_client \
	check_mmux_one_client \
	check_smux_many_clients \
	check_mmux_many_clients \
	check_smux_timeout_after_add \
	check_mmux_timeout_after_add \
	check_smux_timeout_before_add \
	check_mmux_timeout_before_add

check_LTLIBRARIES = libincserver.la

check_PROGRAMS = \
	check_errno_to_string \
	check_fd \
	check_mmux_empty \
	check_mmux_many_clients \
	check_mmux_one_client \
	check_mmux_timeout_after_add \
	check_mmux_timeout_before_add \
	check_smux_empty \
	check_smux_many_clients \
	check_smux_one_client \
	check_smux_timeout_after_add \
	check_smux_timeout_before_add \
	check_split_ipvx_host_port \
	check_tcp_accept_nonblocking \
	check_tcp_listen_and_accept \
	check_tcp_recv_shutdown \
	check_tcp_send_and_recv

check_errno_to_string_SOURCES = check_errno_to_string.cpp
check_errno_to_string_LDADD = $(libcheck) ../libnet.la

check_fd_SOURCES = check_fd.cpp
check_fd_LDADD = $(libcheck) ../libnet.la

check_mmux_empty_SOURCES = check_mmux_empty.cpp
check_mmux_empty_LDADD = $(libcheck) ../libnet.la

check_mmux_many_clients_SOURCES = check_mmux_many_clients.cpp
check_mmux_many_clients_LDADD = $(libcheck) ../libnet.la libincserver.la

check_mmux_one_client_SOURCES = check_mmux_one_client.cpp
check_mmux_one_client_LDADD = $(libcheck) ../libnet.la libincserver.la

check_mmux_timeout_after_add_SOURCES = check_mmux_timeout_after_add.cpp
check_mmux_timeout_after_add_LDADD = $(libcheck) ../libnet.la libincserver.la

check_mmux_timeout_before_add_SOURCES = check_mmux_timeout_before_add.cpp
check_mmux_timeout_before_add_LDADD = $(libcheck) ../libnet.la libincserver.la

check_smux_empty_SOURCES = check_smux_empty.cpp
check_smux_empty_LDADD = $(libcheck) ../libnet.la

check_smux_many_clients_SOURCES = check_smux_many_clients.cpp
check_smux_many_clients_LDADD = $(libcheck) ../libnet.la libincserver.la

check_smux_one_client_SOURCES = check_smux_one_client.cpp
check_smux_one_client_LDADD = $(libcheck) ../libnet.la libincserver.la

check_smux_timeout_after_add_SOURCES = check_smux_timeout_after_add.cpp
check_smux_timeout_after_add_LDADD = $(libcheck) ../libnet.la libincserver.la

check_smux_timeout_before_add_SOURCES = check_smux_timeout_before_add.cpp
check_smux_timeout_before_add_LDADD = $(libcheck) ../libnet.la libincserver.la

check_split_ipvx_host_port_SOURCES = check_split_ipvx_host_port.cpp
check_split_ipvx_host_port_LDADD = $(libcheck)

check_tcp_accept_nonblocking_SOURCES = check_tcp_accept_nonblocking.cpp
check_tcp_accept_nonblocking_LDADD = $(libcheck) ../libnet.la

check_tcp_listen_and_accept_SOURCES = check_tcp_listen_and_accept.cpp
check_tcp_listen_and_accept_LDADD = $(libcheck) ../libnet.la

check_tcp_recv_shutdown_SOURCES = check_tcp_recv_shutdown.cpp
check_tcp_recv_shutdown_LDADD = $(libcheck) ../libnet.la

check_tcp_send_and_recv_SOURCES = check_tcp_send_and_recv.cpp
check_tcp_send_and_recv_LDADD = $(libcheck) ../libnet.la

libincserver_la_SOURCES = inc_server.cpp
libincserver_la_LIBADD = $(libcheck) ../libnet.la

