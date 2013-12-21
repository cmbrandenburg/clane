AM_CXXFLAGS = -Wall -Werror -std=c++11 -I../../check

leak-check: check
	../../leak_check $(TESTS)

TESTS = \
	check_errno_to_string \
	check_fd \
	check_socket_hello_tcp4 \
	check_socket_hello_tcp6 \
	check_socket_hello_tcp \
	check_socket_connect \
	check_socket_accept_nonblock_tcp \
	check_socket_recv_all_tcp \
	check_socket_recv_nonblock_tcp \
	check_socket_send_nonblock_tcp \
	check_smux_run_terminate \
	check_mmux_run_terminate \
	check_smux_one_conn_one_thread \
	check_mmux_one_conn_one_thread \
	check_smux_many_conns_one_thread \
	check_mmux_many_conns_one_thread \
	check_mmux_one_conn_many_threads \
	check_mmux_many_conns_many_threads \
	check_smux_timeout_before_attach \
	check_mmux_timeout_before_attach \
	check_smux_timeout_after_attach \
	check_mmux_timeout_after_attach

check_PROGRAMS = \
	check_errno_to_string \
	check_fd \
	check_mmux_many_conns_many_threads \
	check_mmux_many_conns_one_thread \
	check_mmux_one_conn_many_threads \
	check_mmux_one_conn_one_thread \
	check_mmux_run_terminate \
	check_mmux_timeout_before_attach \
	check_mmux_timeout_after_attach \
	check_smux_many_conns_one_thread \
	check_smux_one_conn_one_thread \
	check_smux_run_terminate \
	check_smux_timeout_before_attach \
	check_smux_timeout_after_attach \
	check_socket_accept_nonblock_tcp \
	check_socket_connect \
	check_socket_hello_tcp4 \
	check_socket_hello_tcp6 \
	check_socket_hello_tcp \
	check_socket_recv_all_tcp \
	check_socket_recv_nonblock_tcp \
	check_socket_send_nonblock_tcp

check_LTLIBRARIES = \
	libcheck_mux_n_conns_n_threads.la \
	libcheck_mux_run_terminate.la \
	libcheck_mux_timeout.la \
	libcheck_socket_hello.la \
	libinc_server.la

check_errno_to_string_SOURCES = check_errno_to_string.cpp
check_errno_to_string_LDADD = ../libnet.la

check_fd_SOURCES = check_fd.cpp
check_fd_LDADD = ../libnet.la

check_mmux_many_conns_many_threads_SOURCES = check_mmux_many_conns_many_threads.cpp
check_mmux_many_conns_many_threads_LDADD = ../libnet.la libcheck_mux_n_conns_n_threads.la

check_mmux_many_conns_one_thread_SOURCES = check_mmux_many_conns_one_thread.cpp
check_mmux_many_conns_one_thread_LDADD = ../libnet.la libcheck_mux_n_conns_n_threads.la

check_mmux_one_conn_many_threads_SOURCES = check_mmux_one_conn_many_threads.cpp
check_mmux_one_conn_many_threads_LDADD = ../libnet.la libcheck_mux_n_conns_n_threads.la

check_mmux_one_conn_one_thread_SOURCES = check_mmux_one_conn_one_thread.cpp
check_mmux_one_conn_one_thread_LDADD = ../libnet.la libcheck_mux_n_conns_n_threads.la

check_mmux_run_terminate_SOURCES = check_mmux_run_terminate.cpp
check_mmux_run_terminate_LDADD = ../libnet.la libcheck_mux_run_terminate.la

check_mmux_timeout_after_attach_SOURCES = check_mmux_timeout_after_attach.cpp
check_mmux_timeout_after_attach_LDADD = ../libnet.la libcheck_mux_timeout.la

check_mmux_timeout_before_attach_SOURCES = check_mmux_timeout_before_attach.cpp
check_mmux_timeout_before_attach_LDADD = ../libnet.la libcheck_mux_timeout.la

check_smux_many_conns_one_thread_SOURCES = check_smux_many_conns_one_thread.cpp
check_smux_many_conns_one_thread_LDADD = ../libnet.la libcheck_mux_n_conns_n_threads.la

check_smux_one_conn_one_thread_SOURCES = check_smux_one_conn_one_thread.cpp
check_smux_one_conn_one_thread_LDADD = ../libnet.la libcheck_mux_n_conns_n_threads.la

check_smux_run_terminate_SOURCES = check_smux_run_terminate.cpp
check_smux_run_terminate_LDADD = ../libnet.la libcheck_mux_run_terminate.la

check_smux_timeout_after_attach_SOURCES = check_smux_timeout_after_attach.cpp
check_smux_timeout_after_attach_LDADD = ../libnet.la libcheck_mux_timeout.la

check_smux_timeout_before_attach_SOURCES = check_smux_timeout_before_attach.cpp
check_smux_timeout_before_attach_LDADD = ../libnet.la libcheck_mux_timeout.la

check_socket_accept_nonblock_tcp_SOURCES = check_socket_accept_nonblock_tcp.cpp
check_socket_accept_nonblock_tcp_LDADD = ../libnet.la

check_socket_connect_SOURCES = check_socket_connect.cpp
check_socket_connect_LDADD = ../libnet.la

check_socket_hello_tcp_SOURCES = check_socket_hello_tcp.cpp
check_socket_hello_tcp_LDADD = ../libnet.la libcheck_socket_hello.la

check_socket_hello_tcp4_SOURCES = check_socket_hello_tcp4.cpp
check_socket_hello_tcp4_LDADD = ../libnet.la libcheck_socket_hello.la

check_socket_hello_tcp6_SOURCES = check_socket_hello_tcp6.cpp
check_socket_hello_tcp6_LDADD = ../libnet.la libcheck_socket_hello.la

check_socket_recv_all_tcp_SOURCES = check_socket_recv_all_tcp.cpp
check_socket_recv_all_tcp_LDADD = ../libnet.la

check_socket_recv_nonblock_tcp_SOURCES = check_socket_recv_nonblock_tcp.cpp
check_socket_recv_nonblock_tcp_LDADD = ../libnet.la

check_socket_send_nonblock_tcp_SOURCES = check_socket_send_nonblock_tcp.cpp
check_socket_send_nonblock_tcp_LDADD = ../libnet.la

libcheck_mux_n_conns_n_threads_la_SOURCES = check_mux_n_conns_n_threads.cpp check_mux_n_conns_n_threads.h
libcheck_mux_n_conns_n_threads_la_LIBADD = ../libnet.la libinc_server.la

libcheck_mux_run_terminate_la_SOURCES = check_mux_run_terminate.cpp check_mux_run_terminate.h
libcheck_mux_run_terminate_la_LIBADD = ../libnet.la

libcheck_mux_timeout_la_SOURCES = check_mux_timeout.cpp check_mux_timeout.h
libcheck_mux_timeout_la_LIBADD = ../libnet.la

libcheck_socket_hello_la_SOURCES = check_socket_hello.cpp check_socket_hello.h
libcheck_socket_hello_la_LIBADD = ../libnet.la

libinc_server_la_SOURCES = inc_server.cpp inc_server.h
libinc_server_la_LIBADD = ../libnet.la

