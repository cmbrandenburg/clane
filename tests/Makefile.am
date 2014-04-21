AM_CXXFLAGS = -Wall -Werror $(BOOST_CPPFLAGS)
AM_LDFLAGS = $(BOOST_LDFLAGS)

noinst_HEADERS = \
	clane_check.hpp \
	clane_http_v1x_response.hpp

TESTS = \
	check_ascii_icase_compare \
	check_ascii_find_newline \
	check_ascii_rtrim \
	check_posix_unique_fd \
	check_sync_wait_group \
	check_net_poll_event \
	check_net_tcp_connect_accept \
	check_net_tcp_connect_accept_nb \
	check_mime_map \
	check_uri_is_percent_encoded \
	check_uri_percent_decode \
	check_uri_percent_encode \
	check_uri_is_scheme \
	check_uri_is_userinfo \
	check_uri_is_ipv4_address \
	check_uri_is_ls32 \
	check_uri_is_ipv6_address \
	check_uri_is_ipvfut_address \
	check_uri_is_reg_name \
	check_uri_is_port \
	check_uri_is_path \
	check_uri_is_query \
	check_uri_is_fragment \
	check_uri_remove_last_path_segment \
	check_uri_remove_dot_segments \
	check_uri_remove_empty_segments \
	check_parse_uri_reference \
	check_uri_validate \
	check_uri_to_string \
	check_http_status_code \
	check_http_header_map \
	check_http_canonize_1x_header_name \
	check_http_is_header_name_valid \
	check_http_is_header_value_valid \
	check_http_is_method_valid \
	check_http_parse_version \
	check_http_parse_status_code \
	check_http_query_headers_chunked \
	check_http_query_headers_content_length \
	check_http_v1x_request_line_incparser \
	check_http_v1x_status_line_incparser \
	check_http_v1x_headers_incparser \
	check_http_v1x_chunk_line_incparser \
	check_http_v1x_body_incparser \
	check_http_v1x_request_incparser \
	check_http_v1x_response_incparser \
	check_http_response_record \
	check_http_default_error_handler \
	check_http_prefix_stripper \
	check_http_serve_file \
	check_http_file_server \
	check_http_route \
	check_http_router \
	check_http_server_run_term \
	check_http_server_term_then_run \
	check_http_server_response \
	check_http_server_error_response \
	check_http_server_content_length_response

check_PROGRAMS =

check_PROGRAMS += check_ascii_find_newline
check_ascii_find_newline_LDADD = ../libclane.la
check_ascii_find_newline_SOURCES = check_ascii_find_newline.cpp

check_PROGRAMS += check_ascii_icase_compare
check_ascii_icase_compare_LDADD = ../libclane.la
check_ascii_icase_compare_SOURCES = check_ascii_icase_compare.cpp

check_PROGRAMS += check_ascii_rtrim
check_ascii_rtrim_LDADD = ../libclane.la
check_ascii_rtrim_SOURCES = check_ascii_rtrim.cpp

check_PROGRAMS += check_http_canonize_1x_header_name
check_http_canonize_1x_header_name_LDADD = ../libclane.la
check_http_canonize_1x_header_name_SOURCES = check_http_canonize_1x_header_name.cpp

check_PROGRAMS += check_http_default_error_handler
check_http_default_error_handler_LDADD = ../libclane.la
check_http_default_error_handler_SOURCES = check_http_default_error_handler.cpp

check_PROGRAMS += check_http_file_server
check_http_file_server_LDADD = ../libclane.la
check_http_file_server_SOURCES = check_http_file_server.cpp

check_PROGRAMS += check_http_header_map
check_http_header_map_LDADD = ../libclane.la
check_http_header_map_SOURCES = check_http_header_map.cpp

check_PROGRAMS += check_http_is_header_name_valid
check_http_is_header_name_valid_LDADD = ../libclane.la
check_http_is_header_name_valid_SOURCES = check_http_is_header_name_valid.cpp

check_PROGRAMS += check_http_is_header_value_valid
check_http_is_header_value_valid_LDADD = ../libclane.la
check_http_is_header_value_valid_SOURCES = check_http_is_header_value_valid.cpp

check_PROGRAMS += check_http_is_method_valid
check_http_is_method_valid_LDADD = ../libclane.la
check_http_is_method_valid_SOURCES = check_http_is_method_valid.cpp

check_PROGRAMS += check_http_parse_status_code
check_http_parse_status_code_LDADD = ../libclane.la
check_http_parse_status_code_SOURCES = check_http_parse_status_code.cpp

check_PROGRAMS += check_http_parse_version
check_http_parse_version_LDADD = ../libclane.la
check_http_parse_version_SOURCES = check_http_parse_version.cpp

check_PROGRAMS += check_http_prefix_stripper
check_http_prefix_stripper_LDADD = ../libclane.la
check_http_prefix_stripper_SOURCES = check_http_prefix_stripper.cpp

check_PROGRAMS += check_http_query_headers_chunked
check_http_query_headers_chunked_LDADD = ../libclane.la
check_http_query_headers_chunked_SOURCES = check_http_query_headers_chunked.cpp

check_PROGRAMS += check_http_query_headers_content_length
check_http_query_headers_content_length_LDADD = ../libclane.la
check_http_query_headers_content_length_SOURCES = check_http_query_headers_content_length.cpp

check_PROGRAMS += check_http_server_response
check_http_server_response_LDADD = ../libclane.la
check_http_server_response_SOURCES = check_http_server_response.cpp

check_PROGRAMS += check_http_response_record
check_http_response_record_LDADD = ../libclane.la
check_http_response_record_SOURCES = check_http_response_record.cpp

check_PROGRAMS += check_http_route
check_http_route_LDADD = ../libclane.la
check_http_route_SOURCES = check_http_route.cpp

check_PROGRAMS += check_http_router
check_http_router_LDADD = ../libclane.la
check_http_router_SOURCES = check_http_router.cpp

check_PROGRAMS += check_http_serve_file
check_http_serve_file_LDADD = ../libclane.la
check_http_serve_file_SOURCES = check_http_serve_file.cpp

check_PROGRAMS += check_http_server_content_length_response
check_http_server_content_length_response_LDADD = ../libclane.la
check_http_server_content_length_response_SOURCES = check_http_server_content_length_response.cpp

check_PROGRAMS += check_http_server_error_response
check_http_server_error_response_LDADD = ../libclane.la
check_http_server_error_response_SOURCES = check_http_server_error_response.cpp

check_PROGRAMS += check_http_server_run_term
check_http_server_run_term_LDADD = ../libclane.la
check_http_server_run_term_SOURCES = check_http_server_run_term.cpp

check_PROGRAMS += check_http_server_term_then_run
check_http_server_term_then_run_LDADD = ../libclane.la
check_http_server_term_then_run_SOURCES = check_http_server_term_then_run.cpp

check_PROGRAMS += check_http_status_code
check_http_status_code_LDADD = ../libclane.la
check_http_status_code_SOURCES = check_http_status_code.cpp

check_PROGRAMS += check_http_v1x_body_incparser
check_http_v1x_body_incparser_LDADD = ../libclane.la
check_http_v1x_body_incparser_SOURCES = check_http_v1x_body_incparser.cpp

check_PROGRAMS += check_http_v1x_chunk_line_incparser
check_http_v1x_chunk_line_incparser_LDADD = ../libclane.la
check_http_v1x_chunk_line_incparser_SOURCES = check_http_v1x_chunk_line_incparser.cpp

check_PROGRAMS += check_http_v1x_headers_incparser
check_http_v1x_headers_incparser_LDADD = ../libclane.la
check_http_v1x_headers_incparser_SOURCES = check_http_v1x_headers_incparser.cpp

check_PROGRAMS += check_http_v1x_request_incparser
check_http_v1x_request_incparser_LDADD = ../libclane.la
check_http_v1x_request_incparser_SOURCES = check_http_v1x_request_incparser.cpp

check_PROGRAMS += check_http_v1x_request_line_incparser
check_http_v1x_request_line_incparser_LDADD = ../libclane.la
check_http_v1x_request_line_incparser_SOURCES = check_http_v1x_request_line_incparser.cpp

check_PROGRAMS += check_http_v1x_response_incparser
check_http_v1x_response_incparser_LDADD = ../libclane.la
check_http_v1x_response_incparser_SOURCES = check_http_v1x_response_incparser.cpp

check_PROGRAMS += check_http_v1x_status_line_incparser
check_http_v1x_status_line_incparser_LDADD = ../libclane.la
check_http_v1x_status_line_incparser_SOURCES = check_http_v1x_status_line_incparser.cpp

check_PROGRAMS += check_mime_map
check_mime_map_LDADD = ../libclane.la
check_mime_map_SOURCES = check_mime_map.cpp

check_PROGRAMS += check_net_poll_event
check_net_poll_event_LDADD = ../libclane.la
check_net_poll_event_SOURCES = check_net_poll_event.cpp

check_PROGRAMS += check_net_tcp_connect_accept
check_net_tcp_connect_accept_LDADD = ../libclane.la
check_net_tcp_connect_accept_SOURCES = check_net_tcp_connect_accept.cpp

check_PROGRAMS += check_net_tcp_connect_accept_nb
check_net_tcp_connect_accept_nb_LDADD = ../libclane.la
check_net_tcp_connect_accept_nb_SOURCES = check_net_tcp_connect_accept_nb.cpp

check_PROGRAMS += check_parse_uri_reference
check_parse_uri_reference_LDADD = ../libclane.la
check_parse_uri_reference_SOURCES = check_parse_uri_reference.cpp

check_PROGRAMS += check_posix_unique_fd
check_posix_unique_fd_LDADD = ../libclane.la
check_posix_unique_fd_SOURCES = check_posix_unique_fd.cpp

check_PROGRAMS += check_sync_wait_group
check_sync_wait_group_LDADD = ../libclane.la
check_sync_wait_group_SOURCES = check_sync_wait_group.cpp

check_PROGRAMS += check_uri_is_fragment
check_uri_is_fragment_LDADD = ../libclane.la
check_uri_is_fragment_SOURCES = check_uri_is_fragment.cpp

check_PROGRAMS += check_uri_is_ipv4_address
check_uri_is_ipv4_address_LDADD = ../libclane.la
check_uri_is_ipv4_address_SOURCES = check_uri_is_ipv4_address.cpp

check_PROGRAMS += check_uri_is_ipv6_address
check_uri_is_ipv6_address_LDADD = ../libclane.la
check_uri_is_ipv6_address_SOURCES = check_uri_is_ipv6_address.cpp

check_PROGRAMS += check_uri_is_ipvfut_address
check_uri_is_ipvfut_address_LDADD = ../libclane.la
check_uri_is_ipvfut_address_SOURCES = check_uri_is_ipvfut_address.cpp

check_PROGRAMS += check_uri_is_ls32
check_uri_is_ls32_LDADD = ../libclane.la
check_uri_is_ls32_SOURCES = check_uri_is_ls32.cpp

check_PROGRAMS += check_uri_is_path
check_uri_is_path_LDADD = ../libclane.la
check_uri_is_path_SOURCES = check_uri_is_path.cpp

check_PROGRAMS += check_uri_is_percent_encoded
check_uri_is_percent_encoded_LDADD = ../libclane.la
check_uri_is_percent_encoded_SOURCES = check_uri_is_percent_encoded.cpp

check_PROGRAMS += check_uri_is_port
check_uri_is_port_LDADD = ../libclane.la
check_uri_is_port_SOURCES = check_uri_is_port.cpp

check_PROGRAMS += check_uri_is_query
check_uri_is_query_LDADD = ../libclane.la
check_uri_is_query_SOURCES = check_uri_is_query.cpp

check_PROGRAMS += check_uri_is_reg_name
check_uri_is_reg_name_LDADD = ../libclane.la
check_uri_is_reg_name_SOURCES = check_uri_is_reg_name.cpp

check_PROGRAMS += check_uri_is_scheme
check_uri_is_scheme_LDADD = ../libclane.la
check_uri_is_scheme_SOURCES = check_uri_is_scheme.cpp

check_PROGRAMS += check_uri_is_userinfo
check_uri_is_userinfo_LDADD = ../libclane.la
check_uri_is_userinfo_SOURCES = check_uri_is_userinfo.cpp

check_PROGRAMS += check_uri_percent_decode
check_uri_percent_decode_LDADD = ../libclane.la
check_uri_percent_decode_SOURCES = check_uri_percent_decode.cpp

check_PROGRAMS += check_uri_percent_encode
check_uri_percent_encode_LDADD = ../libclane.la
check_uri_percent_encode_SOURCES = check_uri_percent_encode.cpp

check_PROGRAMS += check_uri_remove_dot_segments
check_uri_remove_dot_segments_LDADD = ../libclane.la
check_uri_remove_dot_segments_SOURCES = check_uri_remove_dot_segments.cpp

check_PROGRAMS += check_uri_remove_empty_segments
check_uri_remove_empty_segments_LDADD = ../libclane.la
check_uri_remove_empty_segments_SOURCES = check_uri_remove_empty_segments.cpp

check_PROGRAMS += check_uri_remove_last_path_segment
check_uri_remove_last_path_segment_LDADD = ../libclane.la
check_uri_remove_last_path_segment_SOURCES = check_uri_remove_last_path_segment.cpp

check_PROGRAMS += check_uri_validate
check_uri_validate_LDADD = ../libclane.la
check_uri_validate_SOURCES = check_uri_validate.cpp

check_PROGRAMS += check_uri_to_string
check_uri_to_string_LDADD = ../libclane.la
check_uri_to_string_SOURCES = check_uri_to_string.cpp

