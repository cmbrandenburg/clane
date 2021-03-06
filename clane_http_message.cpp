// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// vim: set noet:

/** @file */

#include "clane_http_message.hpp"

namespace clane {
	namespace http {

		char const *what(status_code n) {
			switch (n) {

				// 1xx:
				case status_code::cont: return "Continue";
				case status_code::switching_protocols: return "Switching protocols";

				// 2xx:
				case status_code::ok: return "OK";
				case status_code::created: return "Created";
				case status_code::accepted: return "Accepted";
				case status_code::non_authoritative_information: return "Non-authoritative information";
				case status_code::no_content: return "No content";
				case status_code::reset_content: return "Reset content";
				case status_code::partial_content: return "Partial content";

				// 3xx:
				case status_code::multiple_choices: return "Multiple choices";
				case status_code::moved_permanently: return "Moved permanently";
				case status_code::found: return "Found";
				case status_code::see_other: return "See other";
				case status_code::not_modified: return "Not modified";
				case status_code::use_proxy: return "Use proxy";
				case status_code::temporary_redirect: return "Temporary redirect";

				// 4xx:
				case status_code::bad_request: return "Bad request";
				case status_code::unauthorized: return "Unauthorized";
				case status_code::payment_required: return "Payment required";
				case status_code::forbidden: return "Forbidden";
				case status_code::not_found: return "Not found";
				case status_code::method_not_allowed: return "Method not allowed";
				case status_code::not_acceptable: return "Not acceptable";
				case status_code::proxy_authentication_required: return "Proxy authentication required";
				case status_code::request_timeout: return "Request timeout";
				case status_code::conflict: return "Conflict";
				case status_code::gone: return "Gone";
				case status_code::length_required: return "Length required";
				case status_code::precondition_failed: return "Precondition failed";
				case status_code::request_entity_too_large: return "Request entity too large";
				case status_code::request_uri_too_long: return "Request-URI too long";
				case status_code::unsupported_media_type: return "Unsupported media type";
				case status_code::requested_range_not_satisfiable: return "Requested range not satisfiable";
				case status_code::expectation_failed: return "Expectation failed";

				// 5xx:
				case status_code::internal_server_error: return "Internal server error";
				case status_code::not_implemented: return "Not implemented";
				case status_code::bad_gateway: return "Bad gateway";
				case status_code::service_unavailable: return "Service unavailable";
				case status_code::gateway_timeout: return "Gateway timeout";
				case status_code::http_version_not_supported: return "HTTP version not supported";
			}
			return "";
		}

		void canonize_1x_header_name(char *beg, char *end) {
			bool cap = true;
			char *i = beg;
			while (i < end) {
				if (cap) {
					*i = std::toupper(*i);
					cap = false;
				} else if ('-' == *i) {
					cap = true;
				} else {
					*i = std::tolower(*i);
				}
				++i;
			}
		}

	}
}

