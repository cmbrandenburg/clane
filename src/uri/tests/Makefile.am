AM_CXXFLAGS = -Wall -Werror -std=c++11 -I../../check
libcheck = ../../check/libcheck.la

leak-check: check
	../../leak_check $(TESTS)

TESTS = \
	check_percent_decode \
	check_percent_encode \
	check_is_ipv4_addr \
	check_is_ipv6_addr \
	check_is_ipvfut \
	check_is_ip_literal \
	check_is_port \
	check_is_scheme \
	check_decode_user_info \
	check_decode_host \
	check_decode_path \
	check_decode_query \
	check_decode_fragment \
	check_normalize_path \
	check_parse_uri_reference \
	check_uri_to_string

check_PROGRAMS = \
	check_decode_fragment \
	check_decode_host \
	check_decode_path \
	check_decode_query \
	check_decode_user_info \
	check_is_ipv4_addr \
	check_is_ipv6_addr \
	check_is_ipvfut \
	check_is_ip_literal \
	check_is_port \
	check_is_scheme \
	check_normalize_path \
	check_parse_uri_reference \
	check_percent_decode \
	check_percent_encode \
	check_uri_to_string

check_decode_fragment_SOURCES = check_decode_fragment.cpp
check_decode_fragment_LDADD = $(libcheck) ../liburi.la

check_decode_host_SOURCES = check_decode_host.cpp
check_decode_host_LDADD = $(libcheck) ../liburi.la

check_decode_path_SOURCES = check_decode_path.cpp
check_decode_path_LDADD = $(libcheck) ../liburi.la

check_decode_query_SOURCES = check_decode_query.cpp
check_decode_query_LDADD = $(libcheck) ../liburi.la

check_decode_user_info_SOURCES = check_decode_user_info.cpp
check_decode_user_info_LDADD = $(libcheck) ../liburi.la

check_is_ip_literal_SOURCES = check_is_ip_literal.cpp
check_is_ip_literal_LDADD = $(libcheck) ../liburi.la

check_is_ipv4_addr_SOURCES = check_is_ipv4_addr.cpp
check_is_ipv4_addr_LDADD = $(libcheck) ../liburi.la

check_is_ipv6_addr_SOURCES = check_is_ipv6_addr.cpp
check_is_ipv6_addr_LDADD = $(libcheck) ../liburi.la

check_is_ipvfut_SOURCES = check_is_ipvfut.cpp
check_is_ipvfut_LDADD = $(libcheck) ../liburi.la

check_is_port_SOURCES = check_is_port.cpp
check_is_port_LDADD = $(libcheck) ../liburi.la

check_is_scheme_SOURCES = check_is_scheme.cpp
check_is_scheme_LDADD = $(libcheck) ../liburi.la

check_normalize_path_SOURCES = check_normalize_path.cpp
check_normalize_path_LDADD = $(libcheck) ../liburi.la

check_parse_uri_reference_SOURCES = check_parse_uri_reference.cpp
check_parse_uri_reference_LDADD = $(libcheck) ../liburi.la

check_percent_decode_SOURCES = check_percent_decode.cpp
check_percent_decode_LDADD = $(libcheck) ../liburi.la

check_percent_encode_SOURCES = check_percent_encode.cpp
check_percent_encode_LDADD = $(libcheck) ../liburi.la

check_uri_to_string_SOURCES = check_uri_to_string.cpp
check_uri_to_string_LDADD = $(libcheck) ../liburi.la

