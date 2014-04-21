ACLOCAL_AMFLAGS = -I m4

SUBDIRS = . tests examples

# FIXME: remove '_pub' prefix.
pkginclude_HEADERS = \
	include/clane.hpp \
	include/clane_ascii_pub.hpp \
	include/clane_base_pub.hpp \
	include/clane_http_file.hpp \
	include/clane_http_prefix_stripper.hpp \
	include/clane_http_pub.hpp \
	include/clane_http_route.hpp \
	include/clane_net_pub.hpp \
	include/clane_posix_pub.hpp \
	include/clane_regex.hpp \
	include/clane_sync_pub.hpp \
	include/clane_uri_pub.hpp

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

lib_LTLIBRARIES = libclane.la
libclane_AM_CXXFLAGS = $(BOOST_CPPFLAGS)
libclane_la_LIBADD = \
	$(BOOST_REGEX_LIB) \
	$(BOOST_FILESYSTEM_LIB) \
	$(BOOST_SYSTEM_LIB)
libclane_la_LDFLAGS = \
	-version-info 1:0:0
libclane_la_SOURCES = \
	clane_ascii.hpp \
	clane_base.hpp \
	clane_http_client.cpp \
	clane_http_client.hpp \
	clane_http_file.cpp \
	clane_http_file.hpp \
	clane_http_message.cpp \
	clane_http_message.hpp \
	clane_http_parse.cpp \
	clane_http_parse.hpp \
	clane_http_route.cpp \
	clane_http_route.hpp \
	clane_http_server.cpp \
	clane_http_server.hpp \
	clane_mime.cpp \
	clane_mime.hpp \
	clane_net_error.hpp \
	clane_net_event.cpp \
	clane_net_event.hpp \
	clane_net_inet.cpp \
	clane_net_inet.hpp \
	clane_net_poller.cpp \
	clane_net_poller.hpp \
	clane_net_socket.cpp \
	clane_net_socket.hpp \
	clane_posix_fd.cpp \
	clane_posix_fd.hpp \
	clane_sync_wait_group.cpp \
	clane_sync_wait_group.hpp \
	clane_uri.cpp \
	clane_uri.hpp

