// vim: set noet:

/** @file */

#include "clane/clane_http.hpp"
#include "clane_http_server.hpp"

namespace clane {
	namespace http {

		char const *what(status_code c) {
			switch (c) {
				case status_code::cont:                            return "continue";
				case status_code::switching_protocols:             return "switching protocols";
				case status_code::ok:                              return "OK";
				case status_code::created:                         return "created";
				case status_code::accepted:                        return "accepted";
				case status_code::non_authoritative_information:   return "non-authoritative information";
				case status_code::no_content:                      return "no content";
				case status_code::reset_content:                   return "reset content";
				case status_code::partial_content:                 return "partial content";
				case status_code::multiple_choices:                return "multiple choices";
				case status_code::moved_permanently:               return "moved permanently";
				case status_code::found:                           return "found";
				case status_code::see_other:                       return "see other";
				case status_code::not_modified:                    return "not modified";
				case status_code::use_proxy:                       return "use proxy";
				case status_code::temporary_redirect:              return "temporary redirect";
				case status_code::bad_request:                     return "bad request";
				case status_code::unauthorized:                    return "unauthorized";
				case status_code::payment_required:                return "payment required";
				case status_code::forbidden:                       return "forbidden";
				case status_code::not_found:                       return "not found";
				case status_code::method_not_allowed:              return "method not allowed";
				case status_code::not_acceptable:                  return "not acceptable";
				case status_code::proxy_authentication_required:   return "proxy authentication required";
				case status_code::request_timeout:                 return "request timeout";
				case status_code::conflict:                        return "conflict";
				case status_code::gone:                            return "gone";
				case status_code::length_required:                 return "length required";
				case status_code::precondition_failed:             return "precondition failed";
				case status_code::request_entity_too_long:         return "request entity too long";
				case status_code::request_uri_too_long:            return "request URI too long";
				case status_code::unsupported_media_type:          return "unsupported media type";
				case status_code::requested_range_not_satisfiable: return "requested range not satisfiable";
				case status_code::expectation_failed:              return "expectation failed";
				case status_code::internal_server_error:           return "internal server error";
				case status_code::not_implemented:                 return "not implemented";
				case status_code::bad_gateway:                     return "bad gateway";
				case status_code::service_unavailable:             return "service unavailable";
				case status_code::gateway_timeout:                 return "gateway timeout";
				case status_code::http_version_not_supported:      return "HTTP version not supported";
				default:                                           return "(unknown)";
			}
		}

		std::size_t parse_line(char const *p, std::size_t n, std::size_t max, std::string &oline, bool &ocomplete) {
			ocomplete = false;
			char const *eol = std::find(p, p+n, '\n');
			if (eol == p+n) {
				if (oline.size() + n > max)
					return std::string::npos; // line too long
				oline.append(p, n);
				return n; // incomplete
			}
			if (eol == p) {
				if (!oline.empty() && oline.back() == '\r')
					oline.pop_back();
				ocomplete = true;
				return 1; // complete
			}
			std::size_t m = eol-p - ('\r' == *(eol-1) ? 1 : 0);
			if (oline.size() + m > max)
				return std::string::npos; // line too long
			oline.append(p, m);
			ocomplete = true;
			return 1 + eol-p; // complete
		}

		void serve_error(server_transaction &xact, status_code c) {
			// TODO: implement
		}

		std::size_t request_parser::parse(char const *p, std::size_t n, server_transaction &xact) {
			std::size_t len;
			bool complete;

			while (true) {
				switch (m_stat) {
					case state::begin_request_line: {
						if (0 > (len = parse_line(p, n, 8192, m_cur_line, complete))) {
							serve_error(xact, status_code::bad_request);
							return -1;
						}
						if (0 == len)
							return n;
						// TODO: validate the request line fields
						break;
					}
				}
			}

			return 0; // FIXME
		}

	}
}

