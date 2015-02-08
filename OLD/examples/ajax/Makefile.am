AM_CXXFLAGS = -Wall -Werror $(BOOST_CPPFLAGS)
AM_LDFLAGS = $(BOOST_LDFLAGS)

EXTRA_DIST = \
	index.html \
	main.css \
	main.js

if BUILD_EXAMPLES
noinst_PROGRAMS = ajax
endif
EXTRA_PROGRAMS = ajax
ajax_LDADD = ../../libclane.la
ajax_SOURCES = ajax.cpp

