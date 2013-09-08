SUBDIRS = . tests

AM_CXXFLAGS = -Wall -Werror -std=c++11

noinst_LTLIBRARIES = libnet.la
libnet_la_LDFLAGS = -lpthread
libnet_la_SOURCES = \
	net.h \
	net_common.cpp \
	net_common.h \
	net_error.cpp \
	net_error.h \
	net_fd.cpp \
	net_fd.h \
	net_ip.cpp \
	net_ip.h \
	net_mux.cpp \
	net_mux.h \
	net_signal.cpp \
	net_signal.h \
	net_socket.cpp \
	net_socket.h

EXTRA_DIST = \
	net.cpp

leak-check: libnet.la
	@$(MAKE) -C tests leak-check

