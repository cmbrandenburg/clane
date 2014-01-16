SUBDIRS = . tests

AM_CXXFLAGS = -Wall -Werror -std=c++11

noinst_LTLIBRARIES = libhttp.la
libhttp_la_LIBADD = ../net/libnet.la ../uri/liburi.la
libhttp_la_SOURCES = \
	http_common.hpp \
	http_consume.cpp \
	http_consume.hpp \
	http_header.cpp \
	http_header.hpp \
	http_request.cpp \
	http_request.hpp \
	http_server.cpp \
	http_server.hpp \
	http_status.cpp \
	http_status.hpp
