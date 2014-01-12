SUBDIRS = . tests

AM_CXXFLAGS = -Wall -Werror -std=c++11

noinst_LTLIBRARIES = libnet.la
libnet_la_LDFLAGS = -lpthread
libnet_la_SOURCES = \
	net_common.hpp \
	net_event.cpp \
	net_event.hpp \
	net_inet.cpp \
	net_inet.hpp \
	net_poll.cpp \
	net_poll.hpp \
	net_posix.cpp \
	net_posix.hpp \
	net_socket.cpp \
	net_socket.hpp
