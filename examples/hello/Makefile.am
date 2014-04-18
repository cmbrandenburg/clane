AM_CXXFLAGS = -Wall -Werror $(BOOST_CPPFLAGS)
AM_LDFLAGS = $(BOOST_LDFLAGS)

if BUILD_EXAMPLES
noinst_PROGRAMS = hello
endif
EXTRA_PROGRAMS = hello
hello_LDADD = ../../libclane.la
hello_SOURCES = hello.cpp

