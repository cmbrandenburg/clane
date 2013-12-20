SUBDIRS = . tests

AM_CXXFLAGS = -Wall -Werror -std=c++11

noinst_LTLIBRARIES = liburi.la
liburi_la_LIBADD = ../net/libnet.la
liburi_la_SOURCES = \
	uri.cpp \
	uri.h

leak-check: liburi.la
	@$(MAKE) -C tests leak-check

