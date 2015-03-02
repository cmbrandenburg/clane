ACLOCAL_AMFLAGS = -I m4

AM_CXXFLAGS = -Wall -Werror -std=c++14 -I src

api-ref:
	doxygen doc/Doxyfile

check_LTLIBRARIES =
check_PROGRAMS =
noinst_LTLIBRARIES =

check_LTLIBRARIES += src/check/libclane_check.la
src_check_libclane_check_la_SOURCES = \
	src/check/clane_check.cxx \
	src/check/clane_check.hxx

noinst_LTLIBRARIES += src/http/libclane_http.la
src_http_libclane_http_la_SOURCES = \
	src/http/clane_http_header.cxx \
	src/http/clane_http_header.hxx

noinst_LTLIBRARIES += src/net/libclane_net.la
src_net_libclane_net_la_SOURCES = \
	src/net/clane_uri.cxx \
	src/net/clane_uri.hpp \
	src/net/clane_uri.hxx

noinst_LTLIBRARIES += src/string/libclane_string.la
src_string_libclane_string_la_SOURCES = \
	src/string/clane_vstring.hxx

check_PROGRAMS += src/string/tests/check_vstring
src_string_tests_check_vstring_SOURCES = src/string/tests/check_vstring.cxx
src_string_tests_check_vstring_LDADD = src/check/libclane_check.la src/string/libclane_string.la

