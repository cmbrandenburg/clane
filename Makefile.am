ACLOCAL_AMFLAGS = -I m4

all: doc amalgam

# FIXME: reenable examples
#SUBDIRS = . examples

# FIXME: remove '_pub' prefix.
pkginclude_HEADERS = \
	include/clane_ascii.hpp \
	include/clane_base.hpp \
	include/clane_posix.hpp \
	include/clane_sync.hpp

#	include/clane.hpp \
#	include/clane_http_file.hpp \
#	include/clane_http_prefix_stripper.hpp \
#	include/clane_http_pub.hpp \
#	include/clane_http_route.hpp \
#	include/clane_net_pub.hpp \
#	include/clane_regex.hpp \
#	include/clane_uri_pub.hpp

.PHONY: amalgam
amalgam:
	cd amalgam && ./amalgamate

.PHONY: doc
doc:
	doxygen

.PHONY: idoc
idoc:
	(cat Doxyfile && echo "INPUT += .") | doxygen -

EXTRA_DIST = \
	amalgam/amalgamate \
	m4/ax_boost_base.m4 \
	m4/ax_boost_filesystem.m4 \
	m4/ax_boost_regex.m4 \
	m4/ax_boost_system.m4 \
	m4/ax_check_compile_flag.m4 \
	m4/ax_cxx_compile_stdcxx_11.m4

AM_CXXFLAGS = -Wall -Werror

noinst_LTLIBRARIES =

noinst_LTLIBRARIES += src/posix/libclane_posix.la
src_posix_libclane_posix_la_SOURCES = \
	src/posix/clane_posix_fd.cpp \
	src/posix/clane_posix_fd.hpp

noinst_LTLIBRARIES += src/sync/libclane_sync.la
src_sync_libclane_sync_la_SOURCES = \
	src/sync/clane_sync_wait_group.cpp \
	src/sync/clane_sync_wait_group.hpp

TESTS = \
	src/ascii/tests/check_ascii_icase_compare \
	src/ascii/tests/check_ascii_find_newline \
	src/ascii/tests/check_ascii_rtrim \
	src/posix/tests/check_posix_unique_fd \
	src/sync/tests/check_sync_wait_group

check_PROGRAMS =

check_PROGRAMS += src/ascii/tests/check_ascii_find_newline
src_ascii_tests_check_ascii_find_newline_SOURCES = src/ascii/tests/check_ascii_find_newline.cpp

check_PROGRAMS += src/ascii/tests/check_ascii_icase_compare
src_ascii_tests_check_ascii_icase_compare_SOURCES = src/ascii/tests/check_ascii_icase_compare.cpp

check_PROGRAMS += src/ascii/tests/check_ascii_rtrim
src_ascii_tests_check_ascii_rtrim_SOURCES = src/ascii/tests/check_ascii_rtrim.cpp

check_PROGRAMS += src/posix/tests/check_posix_unique_fd
src_posix_tests_check_posix_unique_fd_LDADD = src/posix/libclane_posix.la
src_posix_tests_check_posix_unique_fd_SOURCES = src/posix/tests/check_posix_unique_fd.cpp

check_PROGRAMS += src/sync/tests/check_sync_wait_group
src_sync_tests_check_sync_wait_group_LDADD = src/sync/libclane_sync.la
src_sync_tests_check_sync_wait_group_SOURCES = src/sync/tests/check_sync_wait_group.cpp

#lib_LTLIBRARIES = libclane.la
#libclane_AM_CXXFLAGS = $(BOOST_CPPFLAGS)
#libclane_la_LIBADD = \
#	$(BOOST_REGEX_LIB) \
#	$(BOOST_FILESYSTEM_LIB) \
#	$(BOOST_SYSTEM_LIB)
#libclane_la_LDFLAGS = \
#	-version-info 1:0:0
#libclane_la_SOURCES = \
#	clane_http_client.cpp \
#	clane_http_client.hpp \
#	clane_http_file.cpp \
#	clane_http_file.hpp \
#	clane_http_message.cpp \
#	clane_http_message.hpp \
#	clane_http_parse.cpp \
#	clane_http_parse.hpp \
#	clane_http_route.cpp \
#	clane_http_route.hpp \
#	clane_http_server.cpp \
#	clane_http_server.hpp \
#	clane_mime.cpp \
#	clane_mime.hpp \
#	clane_net_error.hpp \
#	clane_net_event.cpp \
#	clane_net_event.hpp \
#	clane_net_inet.cpp \
#	clane_net_inet.hpp \
#	clane_net_poller.cpp \
#	clane_net_poller.hpp \
#	clane_net_socket.cpp \
#	clane_net_socket.hpp \
#	clane_uri.cpp \
#	clane_uri.hpp

