ACLOCAL_AMFLAGS = -I m4

all: doc amalgam

SUBDIRS = . examples

pkginclude_HEADERS = \
	include/clane.hpp \
	include/clane_ascii.hpp \
	include/clane_base.hpp \
	include/clane_http_file.hpp \
	include/clane_http_message.hpp \
	include/clane_http_prefix_stripper.hpp \
	include/clane_http_route.hpp \
	include/clane_net.hpp \
	include/clane_posix.hpp \
	include/clane_regex.hpp \
	include/clane_sync.hpp \
	include/clane_uri.hpp

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
	m4/ax_cxx_compile_stdcxx_11.m4 \
	Doxyfile

AM_CXXFLAGS = -Wall -Werror
AM_CXXFLAGS += $(BOOST_CPPFLAGS)

noinst_LTLIBRARIES =
noinst_HEADERS =

noinst_HEADERS += src/ascii/clane_ascii.hpp

noinst_HEADERS += src/clane_base.hpp

noinst_LTLIBRARIES += src/http/libclane_http.la
src_http_libclane_http_la_LIBADD = \
	$(BOOST_REGEX_LIB) \
	src/mime/libclane_mime.la \
	src/net/libclane_net.la \
	src/sync/libclane_sync.la \
	src/uri/libclane_uri.la
src_http_libclane_http_la_SOURCES = \
	src/http/clane_http_client.cpp \
	src/http/clane_http_client.hpp \
	src/http/clane_http_file.cpp \
	src/http/clane_http_file.hpp \
	src/http/clane_http_message.cpp \
	src/http/clane_http_message.hpp \
	src/http/clane_http_parse.cpp \
	src/http/clane_http_parse.hpp \
	src/http/clane_http_prefix_stripper.hpp \
	src/http/clane_http_route.cpp \
	src/http/clane_http_route.hpp \
	src/http/clane_http_server.cpp \
	src/http/clane_http_server.hpp

noinst_LTLIBRARIES += src/mime/libclane_mime.la
src_mime_libclane_mime_la_LIBADD = \
	$(BOOST_FILESYSTEM_LIB) \
	$(BOOST_SYSTEM_LIB)
src_mime_libclane_mime_la_SOURCES = \
	src/mime/clane_mime.cpp \
	src/mime/clane_mime.hpp

noinst_LTLIBRARIES += src/net/libclane_net.la
src_net_libclane_net_la_LIBADD = \
	src/posix/libclane_posix.la \
	-lpthread
src_net_libclane_net_la_SOURCES = \
	src/net/clane_net_connbuf.cpp \
	src/net/clane_net_connbuf.hpp \
	src/net/clane_net_error.hpp \
	src/net/clane_net_event.cpp \
	src/net/clane_net_event.hpp \
	src/net/clane_net_inet.cpp \
	src/net/clane_net_inet.hpp \
	src/net/clane_net_poller.cpp \
	src/net/clane_net_poller.hpp \
	src/net/clane_net_socket.cpp \
	src/net/clane_net_socket.hpp

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

lib_LTLIBRARIES = libclane.la
libclane_la_LDFLAGS = \
	-version-info 1:0:0
libclane_la_LIBADD = \
	src/http/libclane_http.la
libclane_la_SOURCES =

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
	src/uri/tests/check_uri_to_string \
	src/net/tests/check_net_poll_event \
	src/net/tests/check_net_tcp_connect_accept \
	src/net/tests/check_net_tcp_connect_accept_nb \
	src/http/tests/check_http_status_code \
	src/http/tests/check_http_header_map \
	src/http/tests/check_http_canonize_1x_header_name \
	src/http/tests/check_http_is_header_name_valid \
	src/http/tests/check_http_is_header_value_valid \
	src/http/tests/check_http_is_method_valid \
	src/http/tests/check_http_parse_version \
	src/http/tests/check_http_parse_status_code \
	src/http/tests/check_http_query_headers_chunked \
	src/http/tests/check_http_query_headers_content_length \
	src/http/tests/check_http_v1x_request_line_incparser \
	src/http/tests/check_http_v1x_status_line_incparser \
	src/http/tests/check_http_v1x_headers_incparser \
	src/http/tests/check_http_v1x_chunk_line_incparser \
	src/http/tests/check_http_v1x_body_incparser \
	src/http/tests/check_http_v1x_request_incparser \
	src/http/tests/check_http_v1x_response_incparser \
	src/http/tests/check_http_response_record \
	src/http/tests/check_http_prefix_stripper \
	src/http/tests/check_http_serve_file \
	src/http/tests/check_http_file_server \
	src/http/tests/check_http_route \
	src/http/tests/check_http_router \
	src/http/tests/check_http_server_run_term \
	src/http/tests/check_http_server_term_then_run \
	src/http/tests/check_http_server_response \
	src/http/tests/check_http_server_error_response \
	src/http/tests/check_http_server_content_length_response

noinst_HEADERS += src/clane_check.hpp

check_PROGRAMS =

check_PROGRAMS += src/ascii/tests/check_ascii_find_newline
src_ascii_tests_check_ascii_find_newline_SOURCES = src/ascii/tests/check_ascii_find_newline.cpp

check_PROGRAMS += src/ascii/tests/check_ascii_icase_compare
src_ascii_tests_check_ascii_icase_compare_SOURCES = src/ascii/tests/check_ascii_icase_compare.cpp

check_PROGRAMS += src/ascii/tests/check_ascii_rtrim
src_ascii_tests_check_ascii_rtrim_SOURCES = src/ascii/tests/check_ascii_rtrim.cpp

noinst_HEADERS += src/http/tests/clane_http_v1x_response.hpp

check_PROGRAMS += src/http/tests/check_http_canonize_1x_header_name
src_http_tests_check_http_canonize_1x_header_name_LDADD = src/http/libclane_http.la
src_http_tests_check_http_canonize_1x_header_name_SOURCES = src/http/tests/check_http_canonize_1x_header_name.cpp

check_PROGRAMS += src/http/tests/check_http_file_server
src_http_tests_check_http_file_server_LDADD = src/http/libclane_http.la
src_http_tests_check_http_file_server_SOURCES = src/http/tests/check_http_file_server.cpp

check_PROGRAMS += src/http/tests/check_http_header_map
src_http_tests_check_http_header_map_LDADD = src/http/libclane_http.la
src_http_tests_check_http_header_map_SOURCES = src/http/tests/check_http_header_map.cpp

check_PROGRAMS += src/http/tests/check_http_is_header_name_valid
src_http_tests_check_http_is_header_name_valid_LDADD = src/http/libclane_http.la
src_http_tests_check_http_is_header_name_valid_SOURCES = src/http/tests/check_http_is_header_name_valid.cpp

check_PROGRAMS += src/http/tests/check_http_is_header_value_valid
src_http_tests_check_http_is_header_value_valid_LDADD = src/http/libclane_http.la
src_http_tests_check_http_is_header_value_valid_SOURCES = src/http/tests/check_http_is_header_value_valid.cpp

check_PROGRAMS += src/http/tests/check_http_is_method_valid
src_http_tests_check_http_is_method_valid_LDADD = src/http/libclane_http.la
src_http_tests_check_http_is_method_valid_SOURCES = src/http/tests/check_http_is_method_valid.cpp

check_PROGRAMS += src/http/tests/check_http_parse_status_code
src_http_tests_check_http_parse_status_code_LDADD = src/http/libclane_http.la
src_http_tests_check_http_parse_status_code_SOURCES = src/http/tests/check_http_parse_status_code.cpp

check_PROGRAMS += src/http/tests/check_http_parse_version
src_http_tests_check_http_parse_version_LDADD = src/http/libclane_http.la
src_http_tests_check_http_parse_version_SOURCES = src/http/tests/check_http_parse_version.cpp

check_PROGRAMS += src/http/tests/check_http_prefix_stripper
src_http_tests_check_http_prefix_stripper_LDADD = src/http/libclane_http.la
src_http_tests_check_http_prefix_stripper_SOURCES = src/http/tests/check_http_prefix_stripper.cpp

check_PROGRAMS += src/http/tests/check_http_query_headers_chunked
src_http_tests_check_http_query_headers_chunked_LDADD = src/http/libclane_http.la
src_http_tests_check_http_query_headers_chunked_SOURCES = src/http/tests/check_http_query_headers_chunked.cpp

check_PROGRAMS += src/http/tests/check_http_query_headers_content_length
src_http_tests_check_http_query_headers_content_length_LDADD = src/http/libclane_http.la
src_http_tests_check_http_query_headers_content_length_SOURCES = src/http/tests/check_http_query_headers_content_length.cpp

check_PROGRAMS += src/http/tests/check_http_server_response
src_http_tests_check_http_server_response_LDADD = src/http/libclane_http.la
src_http_tests_check_http_server_response_SOURCES = src/http/tests/check_http_server_response.cpp

check_PROGRAMS += src/http/tests/check_http_response_record
src_http_tests_check_http_response_record_LDADD = src/http/libclane_http.la
src_http_tests_check_http_response_record_SOURCES = src/http/tests/check_http_response_record.cpp

check_PROGRAMS += src/http/tests/check_http_route
src_http_tests_check_http_route_LDADD = src/http/libclane_http.la
src_http_tests_check_http_route_SOURCES = src/http/tests/check_http_route.cpp

check_PROGRAMS += src/http/tests/check_http_router
src_http_tests_check_http_router_LDADD = src/http/libclane_http.la
src_http_tests_check_http_router_SOURCES = src/http/tests/check_http_router.cpp

check_PROGRAMS += src/http/tests/check_http_serve_file
src_http_tests_check_http_serve_file_LDADD = src/http/libclane_http.la
src_http_tests_check_http_serve_file_SOURCES = src/http/tests/check_http_serve_file.cpp

check_PROGRAMS += src/http/tests/check_http_server_content_length_response
src_http_tests_check_http_server_content_length_response_LDADD = src/http/libclane_http.la
src_http_tests_check_http_server_content_length_response_SOURCES = src/http/tests/check_http_server_content_length_response.cpp

check_PROGRAMS += src/http/tests/check_http_server_error_response
src_http_tests_check_http_server_error_response_LDADD = src/http/libclane_http.la
src_http_tests_check_http_server_error_response_SOURCES = src/http/tests/check_http_server_error_response.cpp

check_PROGRAMS += src/http/tests/check_http_server_run_term
src_http_tests_check_http_server_run_term_LDADD = src/http/libclane_http.la
src_http_tests_check_http_server_run_term_SOURCES = src/http/tests/check_http_server_run_term.cpp

check_PROGRAMS += src/http/tests/check_http_server_term_then_run
src_http_tests_check_http_server_term_then_run_LDADD = src/http/libclane_http.la
src_http_tests_check_http_server_term_then_run_SOURCES = src/http/tests/check_http_server_term_then_run.cpp

check_PROGRAMS += src/http/tests/check_http_status_code
src_http_tests_check_http_status_code_LDADD = src/http/libclane_http.la
src_http_tests_check_http_status_code_SOURCES = src/http/tests/check_http_status_code.cpp

check_PROGRAMS += src/http/tests/check_http_v1x_body_incparser
src_http_tests_check_http_v1x_body_incparser_LDADD = src/http/libclane_http.la
src_http_tests_check_http_v1x_body_incparser_SOURCES = src/http/tests/check_http_v1x_body_incparser.cpp

check_PROGRAMS += src/http/tests/check_http_v1x_chunk_line_incparser
src_http_tests_check_http_v1x_chunk_line_incparser_LDADD = src/http/libclane_http.la
src_http_tests_check_http_v1x_chunk_line_incparser_SOURCES = src/http/tests/check_http_v1x_chunk_line_incparser.cpp

check_PROGRAMS += src/http/tests/check_http_v1x_headers_incparser
src_http_tests_check_http_v1x_headers_incparser_LDADD = src/http/libclane_http.la
src_http_tests_check_http_v1x_headers_incparser_SOURCES = src/http/tests/check_http_v1x_headers_incparser.cpp

check_PROGRAMS += src/http/tests/check_http_v1x_request_incparser
src_http_tests_check_http_v1x_request_incparser_LDADD = src/http/libclane_http.la
src_http_tests_check_http_v1x_request_incparser_SOURCES = src/http/tests/check_http_v1x_request_incparser.cpp

check_PROGRAMS += src/http/tests/check_http_v1x_request_line_incparser
src_http_tests_check_http_v1x_request_line_incparser_LDADD = src/http/libclane_http.la
src_http_tests_check_http_v1x_request_line_incparser_SOURCES = src/http/tests/check_http_v1x_request_line_incparser.cpp

check_PROGRAMS += src/http/tests/check_http_v1x_response_incparser
src_http_tests_check_http_v1x_response_incparser_LDADD = src/http/libclane_http.la
src_http_tests_check_http_v1x_response_incparser_SOURCES = src/http/tests/check_http_v1x_response_incparser.cpp

check_PROGRAMS += src/http/tests/check_http_v1x_status_line_incparser
src_http_tests_check_http_v1x_status_line_incparser_LDADD = src/http/libclane_http.la
src_http_tests_check_http_v1x_status_line_incparser_SOURCES = src/http/tests/check_http_v1x_status_line_incparser.cpp

check_PROGRAMS += src/mime/tests/check_mime_map
src_mime_tests_check_mime_map_LDADD = src/mime/libclane_mime.la
src_mime_tests_check_mime_map_SOURCES = src/mime/tests/check_mime_map.cpp

check_PROGRAMS += src/net/tests/check_net_poll_event
src_net_tests_check_net_poll_event_LDADD = src/net/libclane_net.la
src_net_tests_check_net_poll_event_SOURCES = src/net/tests/check_net_poll_event.cpp

check_PROGRAMS += src/net/tests/check_net_tcp_connect_accept
src_net_tests_check_net_tcp_connect_accept_LDADD = src/net/libclane_net.la
src_net_tests_check_net_tcp_connect_accept_SOURCES = src/net/tests/check_net_tcp_connect_accept.cpp

check_PROGRAMS += src/net/tests/check_net_tcp_connect_accept_nb
src_net_tests_check_net_tcp_connect_accept_nb_LDADD = src/net/libclane_net.la
src_net_tests_check_net_tcp_connect_accept_nb_SOURCES = src/net/tests/check_net_tcp_connect_accept_nb.cpp

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

