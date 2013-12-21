SUBDIRS = . tests

AM_CXXFLAGS = -Wall -Werror -std=c++11

noinst_LTLIBRARIES = libhttp.la
libhttp_la_LIBADD = ../uri/liburi.la
libhttp_la_SOURCES = \
	http.h \
	http_common.cpp \
	http_common.h \
	http_header.cpp \
	http_header.h \
	http_status.cpp \
	http_status.h

EXTRA_DIST = \
	http.cpp

leak-check: libhttp.la
	@$(MAKE) -C tests leak-check

