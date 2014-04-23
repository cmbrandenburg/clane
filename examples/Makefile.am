AM_CXXFLAGS = -Wall -Werror $(BOOST_CPPFLAGS)
AM_LDFLAGS = $(BOOST_LDFLAGS)

if BUILD_EXAMPLES
noinst_PROGRAMS = \
	ajax/ajax \
	file_server/file_server \
	hello/hello
endif

EXTRA_DIST =
EXTRA_PROGRAMS =

EXTRA_DIST += \
	ajax/index.html \
	ajax/main.css \
	ajax/main.js
EXTRA_PROGRAMS += ajax/ajax
ajax_ajax_LDADD = ../libclane.la
ajax_ajax_SOURCES = ajax/ajax.cpp

EXTRA_PROGRAMS += file_server/file_server
file_server_file_server_LDADD = ../libclane.la
file_server_file_server_SOURCES = file_server/file_server.cpp

EXTRA_PROGRAMS += hello/hello
hello_hello_LDADD = ../libclane.la
hello_hello_SOURCES = hello/hello.cpp

