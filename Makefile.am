ACLOCAL_AMFLAGS = -I m4

all: doc amalgam

# FIXME: reenable examples
#SUBDIRS = . examples

# FIXME: remove '_pub' prefix.
pkginclude_HEADERS = \
	include/clane_ascii.hpp \
	include/clane_base.hpp \
	include/clane_posix.hpp \
	include/clane_sync.hpp

#	include/clane.hpp \
#	include/clane_http_file.hpp \
#	include/clane_http_prefix_stripper.hpp \
#	include/clane_http_pub.hpp \
#	include/clane_http_route.hpp \
#	include/clane_net_pub.hpp \
#	include/clane_regex.hpp \
#	include/clane_uri_pub.hpp

.PHONY: amalgam
amalgam:
	cd amalgam && ./amalgamate

.PHONY: doc
doc:
	doxygen

.PHONY: idoc
idoc:
	(cat Doxyfile && echo "INPUT += .") | doxygen -

EXTRA_DIST = \
	amalgam/amalgamate \
	m4/ax_boost_base.m4 \
	m4/ax_boost_filesystem.m4 \
	m4/ax_boost_regex.m4 \
	m4/ax_boost_system.m4 \
	m4/ax_check_compile_flag.m4 \
	m4/ax_cxx_compile_stdcxx_11.m4

AM_CXXFLAGS = -Wall -Werror

noinst_LTLIBRARIES =

noinst_LTLIBRARIES += src/mime/libclane_mime.la
src_mime_libclane_mime_la_LIBADD = \
	$(BOOST_FILESYSTEM_LIB) \
	$(BOOST_SYSTEM_LIB)
src_mime_libclane_mime_la_SOURCES = \
	src/mime/clane_mime.cpp \
	src/mime/clane_mime.hpp

noinst_LTLIBRARIES += src/posix/libclane_posix.la
src_posix_libclane_posix_la_SOURCES = \
	src/posix/clane_posix_fd.cpp \
	src/posix/clane_posix_fd.hpp

noinst_LTLIBRARIES += src/sync/libclane_sync.la
src_sync_libclane_sync_la_SOURCES = \
	src/sync/clane_sync_wait_group.cpp \
	src/sync/clane_sync_wait_group.hpp

noinst_LTLIBRARIES += src/uri/libclane_uri.la
src_uri_libclane_uri_la_SOURCES = \
	src/uri/clane_uri.cpp \
	src/uri/clane_uri.hpp

TESTS = \
	src/ascii/tests/check_ascii_icase_compare \
	src/ascii/tests/check_ascii_find_newline \
	src/ascii/tests/check_ascii_rtrim \
	src/posix/tests/check_posix_unique_fd \
	src/sync/tests/check_sync_wait_group \
	src/mime/tests/check_mime_map \
	src/uri/tests/check_uri_is_percent_encoded \
	src/uri/tests/check_uri_percent_decode \
	src/uri/tests/check_uri_percent_encode \
	src/uri/tests/check_uri_is_scheme \
	src/uri/tests/check_uri_is_userinfo \
	src/uri/tests/check_uri_is_ipv4_address \
	src/uri/tests/check_uri_is_ls32 \
	src/uri/tests/check_uri_is_ipv6_address \
	src/uri/tests/check_uri_is_ipvfut_address \
	src/uri/tests/check_uri_is_reg_name \
	src/uri/tests/check_uri_is_port \
	src/uri/tests/check_uri_is_path \
	src/uri/tests/check_uri_is_query \
	src/uri/tests/check_uri_is_fragment \
	src/uri/tests/check_uri_remove_last_path_segment \
	src/uri/tests/check_uri_remove_dot_segments \
	src/uri/tests/check_uri_remove_empty_segments \
	src/uri/tests/check_uri_parse_uri_reference \
	src/uri/tests/check_uri_validate \
	src/uri/tests/check_uri_to_string

check_PROGRAMS =

check_PROGRAMS += src/ascii/tests/check_ascii_find_newline
src_ascii_tests_check_ascii_find_newline_SOURCES = src/ascii/tests/check_ascii_find_newline.cpp

check_PROGRAMS += src/ascii/tests/check_ascii_icase_compare
src_ascii_tests_check_ascii_icase_compare_SOURCES = src/ascii/tests/check_ascii_icase_compare.cpp

check_PROGRAMS += src/ascii/tests/check_ascii_rtrim
src_ascii_tests_check_ascii_rtrim_SOURCES = src/ascii/tests/check_ascii_rtrim.cpp

check_PROGRAMS += src/mime/tests/check_mime_map
src_mime_tests_check_mime_map_LDADD = src/mime/libclane_mime.la
src_mime_tests_check_mime_map_SOURCES = src/mime/tests/check_mime_map.cpp

check_PROGRAMS += src/posix/tests/check_posix_unique_fd
src_posix_tests_check_posix_unique_fd_LDADD = src/posix/libclane_posix.la
src_posix_tests_check_posix_unique_fd_SOURCES = src/posix/tests/check_posix_unique_fd.cpp

check_PROGRAMS += src/sync/tests/check_sync_wait_group
src_sync_tests_check_sync_wait_group_LDADD = src/sync/libclane_sync.la
src_sync_tests_check_sync_wait_group_SOURCES = src/sync/tests/check_sync_wait_group.cpp

check_PROGRAMS += src/uri/tests/check_uri_is_fragment
src_uri_tests_check_uri_is_fragment_LDADD = src/uri/libclane_uri.la
src_uri_tests_check_uri_is_fragment_SOURCES = src/uri/tests/check_uri_is_fragment.cpp

check_PROGRAMS += src/uri/tests/check_uri_is_ipv4_address
src_uri_tests_check_uri_is_ipv4_address_LDADD = src/uri/libclane_uri.la
src_uri_tests_check_uri_is_ipv4_address_SOURCES = src/uri/tests/check_uri_is_ipv4_address.cpp

check_PROGRAMS += src/uri/tests/check_uri_is_ipv6_address
src_uri_tests_check_uri_is_ipv6_address_LDADD = src/uri/libclane_uri.la
src_uri_tests_check_uri_is_ipv6_address_SOURCES = src/uri/tests/check_uri_is_ipv6_address.cpp

check_PROGRAMS += src/uri/tests/check_uri_is_ipvfut_address
src_uri_tests_check_uri_is_ipvfut_address_LDADD = src/uri/libclane_uri.la
src_uri_tests_check_uri_is_ipvfut_address_SOURCES = src/uri/tests/check_uri_is_ipvfut_address.cpp

check_PROGRAMS += src/uri/tests/check_uri_is_ls32
src_uri_tests_check_uri_is_ls32_LDADD = src/uri/libclane_uri.la
src_uri_tests_check_uri_is_ls32_SOURCES = src/uri/tests/check_uri_is_ls32.cpp

check_PROGRAMS += src/uri/tests/check_uri_is_path
src_uri_tests_check_uri_is_path_LDADD = src/uri/libclane_uri.la
src_uri_tests_check_uri_is_path_SOURCES = src/uri/tests/check_uri_is_path.cpp

check_PROGRAMS += src/uri/tests/check_uri_is_percent_encoded
src_uri_tests_check_uri_is_percent_encoded_LDADD = src/uri/libclane_uri.la
src_uri_tests_check_uri_is_percent_encoded_SOURCES = src/uri/tests/check_uri_is_percent_encoded.cpp

check_PROGRAMS += src/uri/tests/check_uri_is_port
src_uri_tests_check_uri_is_port_LDADD = src/uri/libclane_uri.la
src_uri_tests_check_uri_is_port_SOURCES = src/uri/tests/check_uri_is_port.cpp

check_PROGRAMS += src/uri/tests/check_uri_is_query
src_uri_tests_check_uri_is_query_LDADD = src/uri/libclane_uri.la
src_uri_tests_check_uri_is_query_SOURCES = src/uri/tests/check_uri_is_query.cpp

check_PROGRAMS += src/uri/tests/check_uri_is_reg_name
src_uri_tests_check_uri_is_reg_name_LDADD = src/uri/libclane_uri.la
src_uri_tests_check_uri_is_reg_name_SOURCES = src/uri/tests/check_uri_is_reg_name.cpp

check_PROGRAMS += src/uri/tests/check_uri_is_scheme
src_uri_tests_check_uri_is_scheme_LDADD = src/uri/libclane_uri.la
src_uri_tests_check_uri_is_scheme_SOURCES = src/uri/tests/check_uri_is_scheme.cpp

check_PROGRAMS += src/uri/tests/check_uri_is_userinfo
src_uri_tests_check_uri_is_userinfo_LDADD = src/uri/libclane_uri.la
src_uri_tests_check_uri_is_userinfo_SOURCES = src/uri/tests/check_uri_is_userinfo.cpp

check_PROGRAMS += src/uri/tests/check_uri_parse_uri_reference
src_uri_tests_check_uri_parse_uri_reference_LDADD = src/uri/libclane_uri.la
src_uri_tests_check_uri_parse_uri_reference_SOURCES = src/uri/tests/check_uri_parse_uri_reference.cpp

check_PROGRAMS += src/uri/tests/check_uri_percent_decode
src_uri_tests_check_uri_percent_decode_LDADD = src/uri/libclane_uri.la
src_uri_tests_check_uri_percent_decode_SOURCES = src/uri/tests/check_uri_percent_decode.cpp

check_PROGRAMS += src/uri/tests/check_uri_percent_encode
src_uri_tests_check_uri_percent_encode_LDADD = src/uri/libclane_uri.la
src_uri_tests_check_uri_percent_encode_SOURCES = src/uri/tests/check_uri_percent_encode.cpp

check_PROGRAMS += src/uri/tests/check_uri_remove_dot_segments
src_uri_tests_check_uri_remove_dot_segments_LDADD = src/uri/libclane_uri.la
src_uri_tests_check_uri_remove_dot_segments_SOURCES = src/uri/tests/check_uri_remove_dot_segments.cpp

check_PROGRAMS += src/uri/tests/check_uri_remove_empty_segments
src_uri_tests_check_uri_remove_empty_segments_LDADD = src/uri/libclane_uri.la
src_uri_tests_check_uri_remove_empty_segments_SOURCES = src/uri/tests/check_uri_remove_empty_segments.cpp

check_PROGRAMS += src/uri/tests/check_uri_remove_last_path_segment
src_uri_tests_check_uri_remove_last_path_segment_LDADD = src/uri/libclane_uri.la
src_uri_tests_check_uri_remove_last_path_segment_SOURCES = src/uri/tests/check_uri_remove_last_path_segment.cpp

check_PROGRAMS += src/uri/tests/check_uri_validate
src_uri_tests_check_uri_validate_LDADD = src/uri/libclane_uri.la
src_uri_tests_check_uri_validate_SOURCES = src/uri/tests/check_uri_validate.cpp

check_PROGRAMS += src/uri/tests/check_uri_to_string
src_uri_tests_check_uri_to_string_LDADD = src/uri/libclane_uri.la
src_uri_tests_check_uri_to_string_SOURCES = src/uri/tests/check_uri_to_string.cpp

#lib_LTLIBRARIES = libclane.la
#libclane_AM_CXXFLAGS = $(BOOST_CPPFLAGS)
#libclane_la_LIBADD = \
#	$(BOOST_REGEX_LIB) \
#	$(BOOST_FILESYSTEM_LIB) \
#	$(BOOST_SYSTEM_LIB)
#libclane_la_LDFLAGS = \
#	-version-info 1:0:0
#libclane_la_SOURCES = \
#	clane_http_client.cpp \
#	clane_http_client.hpp \
#	clane_http_file.cpp \
#	clane_http_file.hpp \
#	clane_http_message.cpp \
#	clane_http_message.hpp \
#	clane_http_parse.cpp \
#	clane_http_parse.hpp \
#	clane_http_route.cpp \
#	clane_http_route.hpp \
#	clane_http_server.cpp \
#	clane_http_server.hpp \
#	clane_net_error.hpp \
#	clane_net_event.cpp \
#	clane_net_event.hpp \
#	clane_net_inet.cpp \
#	clane_net_inet.hpp \
#	clane_net_poller.cpp \
#	clane_net_poller.hpp \
#	clane_net_socket.cpp \
#	clane_net_socket.hpp

