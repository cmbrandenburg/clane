// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// vim: set noet:

/** @file */

#include "ascii/clane_ascii_impl.hpp"
#include "clane/clane_http.hpp"
#include "clane_http_message.hpp"

namespace {

	bool is_control_char(char c) {
		// CTL = <any US-ASCII control character
		//       (octets 0 - 31) and DEL (127)>
		return (0 <= c && c < 32) || c == 127;
	}

	bool is_token_char(char c) {
		// token      = 1*<any CHAR except CTLs or separators>
		// separators = "(" | ")" | "<" | ">" | "@"
		//            | "," | ";" | ":" | "\" | <">
		//            | "/" | "[" | "]" | "?" | "="
		//            | "{" | "}" | SP | HT
		//
		return !is_control_char(c) &&
			'(' != c && ')' != c && '<' != c && '>'  != c && '@' != c &&
			',' != c && ';' != c && ':' != c && '\\' != c && '"' != c &&
			'/' != c && '[' != c && ']' != c && '?'  != c && '=' != c &&
			'{' != c && '}' != c && ' ' != c && '\t' != c;
	}

	bool is_token(char const *beg, char const *end) {
		// token      = 1*<any CHAR except CTLs or separators>
		return beg<end && end == std::find_if_not(beg, end, is_token_char);
	}
}

namespace clane {
	namespace http {

		char const *what(status_code c) {
			switch (c) {
				case status_code::xcontinue:                       return "continue";
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

		bool is_method(char const *beg, char const *end) {
			return is_token(beg, end);
		}

		bool parse_http_version(char const *beg, char const *end, unsigned &omajor, unsigned &ominor) {

			// HTTP-Version = "HTTP" "/" 1*DIGIT "." 1*DIGIT

			static char const *prefix = "HTTP/";
			char const *p = beg;
			auto parse_number = [&p, end](unsigned &o) -> bool {
				if (p == end || !std::isdigit(*p))
					return false; // need at least one digit
				do {
					if (10*o < o)
						return false; // overflow
					o *= 10;
					o += *p - '0';
					++p;
				} while (p != end && std::isdigit(*p));
				return true;
			};

			if (!ascii::has_prefix(p, end, prefix))
				return false;
			p += std::strlen(prefix);
			unsigned major = 0, minor = 0;
			if (!parse_number(major))
				return false;
			if (p == end || *p != '.')
				return false;
			++p;
			if (!parse_number(minor))
				return false;
			omajor = major;
			ominor = minor;
			return true;
		}

#if 0 // FIXME

		std::size_t parse_line(char const *p, std::size_t n, std::size_t max, std::string &oline, bool &ocomplete) {
			ocomplete = false;
			char const *eol = std::find(p, p+n, '\n');
			if (eol == p+n) {
				if (oline.size() + n > max)
					return 0; // line too long
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
				return 0; // line too long
			oline.append(p, m);
			ocomplete = true;
			return 1 + eol-p; // complete
		}

#endif // #if 0

		void v1x_headers_parser::reset() {
			m_phase = phase::begin_line;
			m_hdrs.clear();
			m_cur.clear();
			m_size = 0;
		}

		parse_result v1x_headers_parser::parse(char const *p, std::size_t n) {
			std::size_t off = 0;
			while (n) {
				std::size_t len = std::min(cap-m_size, n-off);
				switch (m_phase) {
					case phase::begin_line: {
						if (ascii::has_prefix(p+off, p+off+len, "\n")) {
							m_hdrs.insert(std::move(m_cur));
							return parse_result{parse_result::done, 1};
						}
						if (ascii::has_prefix(p+off, p+off+len, "\r\n")) {
							m_hdrs.insert(std::move(m_cur));
							return parse_result{parse_result::done, 2};
						}
					case phase::continue_name:
						auto sep = std::find(p+off, p+off+len, ':');
						if (
						m_size += sep-(p+off);
						if (m_size == cap)
							return parse_result{parse_result::error, 0, status_code::bad_request}; // too long
						m_cur.name.append(p+off, sep);
						if (!is_token(m_cur.name.data(), m_cur.name.data()+m_cur.name.size()))
							return parse_result{parse_result::error, 0, status_code::bad_request}; // bad name token
						if (sep == p+off+len) {
							m_phase = phase::continue_name;
							return parse_result{parse_result::not_done, len};
						}
						++m_size; // consume ':'
						m_phase = phase::continue_value;
						break;
					}

					case phase::continue_value: {
						auto sep = std::find(p, p+len, '\n');
						m_size += sep-p;
						if (m_size == cap)
							return parse_result{parse_result::error, 0, status_code::bad_request}; // too long
						if (*
						m_cur.value.append(p, sep);
					}

					default:
						throw std::logic_error("bad parser state");
				}
			}
			return parse_result{parse_result::not_done, n
		}

	}
}

