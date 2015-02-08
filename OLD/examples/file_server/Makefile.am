AM_CXXFLAGS = -Wall -Werror $(BOOST_CPPFLAGS)
AM_LDFLAGS = $(BOOST_LDFLAGS)

if BUILD_EXAMPLES
noinst_PROGRAMS = file_server
endif
EXTRA_PROGRAMS = file_server
file_server_LDADD = ../../libclane.la
file_server_SOURCES = file_server.cpp

