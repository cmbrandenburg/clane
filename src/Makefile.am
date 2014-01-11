SUBDIRS = . tests

AM_CXXFLAGS = -Wall -Werror -std=c++11

lib_LTLIBRARIES = libclane.la
libclane_la_LDFLAGS = -version-info 0:0:0 -lpthread
libclane_la_SOURCES = \
	clane_common.hpp \
	clane_http_server.cpp \
	clane_http_server.hpp \
	clane_inet.cpp \
	clane_inet.hpp \
	clane_posix.cpp \
	clane_posix.hpp \
	clane_socket.cpp \
	clane_socket.hpp

