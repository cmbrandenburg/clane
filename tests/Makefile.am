AM_CXXFLAGS = -Wall -Werror $(BOOST_CPPFLAGS)
AM_LDFLAGS = $(BOOST_LDFLAGS)

noinst_HEADERS = \
	clane_http_v1x_response.hpp

TESTS = \
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

