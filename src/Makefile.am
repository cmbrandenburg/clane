SUBDIRS = . tests

AM_CXXFLAGS = -Wall -Werror -std=c++11

lib_LTLIBRARIES = libclane.la
libclane_la_LDFLAGS = -version-info 0:0:0 -lpthread
libclane_la_SOURCES = \
	clane_common.hpp \
	clane_event.cpp \
	clane_event.hpp \
	clane_http_header.cpp \
	clane_http_header.hpp \
	clane_http_request.cpp \
	clane_http_request.hpp \
	clane_http_server.cpp \
	clane_http_server.hpp \
	clane_inet.cpp \
	clane_inet.hpp \
	clane_poll.cpp \
	clane_poll.hpp \
	clane_posix.cpp \
	clane_posix.hpp \
	clane_socket.cpp \
	clane_socket.hpp

