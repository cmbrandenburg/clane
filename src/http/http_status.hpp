// vim: set noet:

#ifndef CLANE__HTTP_STATUS_H
#define CLANE__HTTP_STATUS_H

#include "http_common.hpp"
#include <ostream>

namespace clane {
	namespace http {

		enum class status_code {

			cont = 100,
			switching_protocols = 101,

			ok = 200,
			created = 201,
			accepted = 202,
			non_authoritative_information = 203,
			no_content = 204,
			reset_content = 205,
			partial_content = 206,

			multiple_choices = 300,
			moved_permanently = 301,
			found = 302,
			see_other = 303,
			not_modified = 304,
			use_proxy = 305,
			temporary_redirect = 307,

			bad_request = 400,
			unauthorized = 401,
			payment_required = 402,
			forbidden = 403,
			not_found = 404,
			method_not_allowed = 405,
			not_acceptable = 406,
			proxy_authentication_required = 407,
			request_timeout = 408,
			conflict = 409,
			gone = 410,
			length_required = 411,
			precondition_failed = 412,
			request_entity_too_large = 413,
			request_uri_too_long = 414,
			unsupported_media_type = 415,
			requested_range_not_satisfiable = 416,
			expectation_failed = 417,

			internal_server_error = 500,
			not_implemented = 501,
			bad_gateway = 502,
			service_unavailable = 503,
			gateway_timeout = 504,
			http_version_not_supported = 505
		};

		std::ostream &operator<<(std::ostream &ostrm, status_code n);

		inline std::ostream &operator<<(std::ostream &ostrm, status_code n) {
#ifndef _WIN32
			return ostrm << static_cast<typename std::underlying_type<status_code>::type>(n);
#else
			return ostrm << static_cast<std::underlying_type<status_code>::type>(n);
#endif
		}

		char const *what(status_code n);

		bool status_code_from_int(status_code &stat, int n);
	}
}

#endif // #ifndef CLANE__HTTP_STATUS_H
