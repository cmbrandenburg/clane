// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// vim: set noet:

/** @file */

#include "ascii/clane_ascii.hxx"
#include "clane/clane_http_message.hpp"
#include "clane_http_message.hxx"
#include <algorithm>
#include <iterator>

namespace {

	bool is_control_char(char c) {
		// CTL = <any US-ASCII control character
		//       (octets 0 - 31) and DEL (127)>
		return (0 <= c && c < 32) || c == 127;
	}

	bool is_lws_char(char c) {
		return c == ' ' || c == '\t';
	};

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

	bool is_header_name(char const *beg, char const *end) {
		// field-name     = token
		return is_token(beg, end);
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

		bool is_header_value(char const *beg, char const *end) {
			// field-value    = *( field-content | LWS )
			// field-content  = <the OCTETs making up the field-value
			//                  and consisting of either *TEXT or combinations
			//                  of token, separators, and quoted-string>
			// TEXT           = <any OCTET except CTLs,
			//                  but including LWS>
			return is_text(beg, end);
		}

		bool is_method(char const *beg, char const *end) {
			return is_token(beg, end);
		}

		bool is_text(char const *beg, char const *end) {
			// TEXT           = <any OCTET except CTLs,
			//                  but including LWS>
			return std::all_of(beg, end, [](char c) { return (32<=c && c<127) || is_lws_char(c); });
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

		void v1x_headers_parser::reset_derived() {
			m_cur_line.clear();
			m_cur_hdr.clear();
			m_hdrs.clear();
		}

		void v1x_headers_parser::parse_some(char const *const p, std::size_t const n) {
			// This parser could detect errors sooner than it does. Instead, the
			// parser waits to read in the entire line, or two entire lines, before
			// detecting whether, say, a header name or value is invalid. The
			// expectation is that clients send their headers quickly, and it's not
			// our responsibility to return an error to the client as soon as possible
			// anyway.
			std::size_t len = std::min(capacity()-size(), n);
			char const *beg, *end;
			auto const eol = std::find(p, p+len, '\n');
			if (eol == p+len) {
				// This is an incomplete line.
				if (!increase_size(eol - p, true))
					return set_bad(status_code_type::bad_request); // headers are too long
				m_cur_line.append(p, eol);
				return;
			}
			size_t const delta = eol+1 - p;
			if (!increase_size(delta, false))
				return set_bad(status_code_type::bad_request); // headers are too long
			std::string cur_line = std::move(m_cur_line);
			m_cur_line.clear();
			if (!cur_line.empty()) {
				// Special case: this parse invocation is a continuation of the same
				// line, so the line is dis-contiguous. Buffer the line into a
				// contiguous buffer to make it easier to work with.
				cur_line.append(p, eol);
				beg = cur_line.data();
				end = cur_line.data()+cur_line.size();
			} else {
				// Normal case: we've received the line in its entirety in one pass.
				beg = p;
				end = eol;
			}
			if (beg < end && *(end-1) == '\r')
				--end; // chomp CR
			if (beg == end || !is_lws_char(*beg)) {
				if (!m_cur_hdr.name.empty()) {
					// We store a header only after we know that the next line doesn't
					// continue the header with linear whitespace at beginning of the
					// line.
					if (!is_header_value(&*begin(m_cur_hdr.value), &*std::end(m_cur_hdr.value)))
						return set_bad(status_code_type::bad_request); // invalid header value
					m_hdrs.insert(std::move(m_cur_hdr));
					m_cur_hdr.clear();
				}
			}
			if (beg == end)
				return set_fin(); // no more headers
			if (is_lws_char(*beg)) {
				// Linear whitespace: this line is a continuation of the previous
				// line.
				if (m_cur_hdr.name.empty())
					return set_bad(status_code_type::bad_request); // missing header name
				beg = std::find_if_not(beg, end, is_lws_char); // skip leading linear whitespace
				end = &*std::find_if_not(std::reverse_iterator<char const*>{end}, std::reverse_iterator<char const *>{beg},
						is_lws_char) + 1; // skip trailing linear whitespace
				if (beg < end) {
					m_cur_hdr.value.push_back(' '); // replace all linear whitespace with single space character
					m_cur_hdr.value.append(beg, end);
				}
				return;
			}
			// Otherwise this line begins a new header.
			auto const sep = std::find(beg, end, ':');
			if (sep == end)
				return set_bad(status_code_type::bad_request); // missing ':' separator
			if (!is_header_name(beg, sep))
				return set_bad(status_code_type::bad_request); // invalid header name
			m_cur_hdr.name.assign(beg, sep);
			beg = std::find_if_not(sep+1, end, is_lws_char); // skip leading linear whitespace
			end = &*std::find_if_not(std::reverse_iterator<char const*>{end}, std::reverse_iterator<char const *>{beg},
					is_lws_char) + 1; // skip trailing linear whitespace
			m_cur_hdr.value.assign(beg, end);
		}

		void v1x_request_line_parser::reset_derived() {
			m_cur_line.clear();
		}

		void v1x_request_line_parser::parse_some(char const *p, std::size_t n) {
		}
	}
}

