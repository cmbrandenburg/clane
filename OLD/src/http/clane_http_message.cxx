// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// vim: set noet:

/** @file */

#include "ascii/clane_ascii.hxx"
#include "clane_http_message.hpp"
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

		bool parse_chunk_size(char const *beg, char const *end, std::size_t *osize) {
			assert(beg <= end);
			if (beg==end || !std::all_of(beg, end, [](char c) { return std::isxdigit(c); }))
				return false;
			if (static_cast<std::size_t>(end-beg) > sizeof(std::size_t)*2)
				return false; // overflow
			std::size_t x = 0;
			for (auto i = beg; i < end; ++i) {
				x *= 16;
				x += ascii::hexch_to_int(*i);
			}
			*osize = x;
			return true;
		}

		bool parse_http_version(char const *beg, char const *end, protocol_version *o_ver) {

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
			o_ver->major = major;
			o_ver->minor = minor;
			return true;
		}

		template <typename Derived> std::size_t parser<Derived>::parse(char const *p, std::size_t n) {
			assert(!bad());
			assert(!fin());
			std::size_t tot = 0;
			while (tot < n) {
				std::size_t const cap = static_cast<Derived*>(this)->capacity();
				std::size_t const len = cap ? std::min(cap-m_size, n-tot) : n-tot;
				std::size_t const x = static_cast<Derived*>(this)->parse_some(p+tot, len);
				if (bad())
					return 0;
				tot += x;
				m_size += x;
				if (fin())
					break;
				if (m_size == cap)
					return mark_bad(status_code_type::bad_request); // size limit reached without being finished
			}
			return tot;
		}

		template <typename Derived> std::size_t v1x_line_parser<Derived>::parse_some(char const *p, std::size_t n) {
			char const *const eol = std::find(p, p+n, '\n');
			if (eol == p+n) {
				// This is an incomplete line.
				m_cur_line.append(p, eol);
				return n;
			}
			char const *beg, *end;
			std::string cur_line = std::move(m_cur_line);
			m_cur_line.clear();
			if (!cur_line.empty()) {
				// Special case: This input buffer completes a line started in a
				// different buffer. Buffer the line into a contiguous buffer to make it
				// easier to work with.
				cur_line.append(p, eol);
				beg = &cur_line[0];
				end = &cur_line[cur_line.size()];
			} else {
				// Normal case: This input buffer contains the line in its entirety.
				beg = p;
				end = eol;
			}
			if (beg < end && *(end-1) == '\r')
				--end; // chomp CR
			// We don't use the return value from the derived class's parse_some()
			// function, but check it with some asserts for consistency with the
			// overall parse_some() pattern.
			static_cast<Derived*>(this)->parse_line(beg, end-beg);
			if (parser<v1x_line_parser>::bad())
				return 0;
			return eol+1 - p;
		}

		std::size_t v1x_empty_line_parser::parse_line(char const *p, std::size_t n) {
			if (n)
				return mark_bad(status_code_type::bad_request);
			return mark_fin(n);
		}

		std::size_t v1x_chunk_size_parser::parse_line(char const *p, std::size_t n) {
			if (!parse_chunk_size(p, p+n, m_chunk_size))
				return mark_bad(status_code_type::bad_request); // invalid chunk size
			return mark_fin(n);
		}

		std::size_t v1x_headers_parser::parse_line(char const *const p, std::size_t const n) {
			// This parser could detect errors sooner than it does. Instead, the
			// parser waits to read in the entire line, or two entire lines, before
			// detecting whether, say, a header name or value is invalid. The
			// expectation is that clients send their headers quickly, and it's not
			// our responsibility to return an error to the client as soon as possible
			// anyway.
			char const *beg = p, *end = p+n;
			if (beg == end || !is_lws_char(*beg)) {
				if (!m_cur_hdr.name.empty()) {
					// We store a header only after we know that the next line doesn't
					// continue the header with linear whitespace at beginning of the
					// line.
					if (!is_header_value(&*begin(m_cur_hdr.value), &*std::end(m_cur_hdr.value)))
						return mark_bad(status_code_type::bad_request); // invalid header value
					m_hdrs->insert(std::move(m_cur_hdr));
					m_cur_hdr.clear();
				}
			}
			if (beg == end)
				return mark_fin(n); // no more headers
			if (is_lws_char(*beg)) {
				// Linear whitespace: this line is a continuation of the previous
				// line.
				if (m_cur_hdr.name.empty())
					return mark_bad(status_code_type::bad_request); // missing header name
				beg = std::find_if_not(beg, end, is_lws_char); // skip leading linear whitespace
				end = &*std::find_if_not(std::reverse_iterator<char const*>{end}, std::reverse_iterator<char const *>{beg},
						is_lws_char) + 1; // skip trailing linear whitespace
				if (beg < end) {
					m_cur_hdr.value.push_back(' '); // replace all linear whitespace with single space character
					m_cur_hdr.value.append(beg, end);
				}
				return n;
			}
			// Otherwise this line begins a new header.
			auto const sep = std::find(beg, end, ':');
			if (sep == end)
				return mark_bad(status_code_type::bad_request); // missing ':' separator
			if (!is_header_name(beg, sep))
				return mark_bad(status_code_type::bad_request); // invalid header name
			m_cur_hdr.name.assign(beg, sep);
			beg = std::find_if_not(sep+1, end, is_lws_char); // skip leading linear whitespace
			end = &*std::find_if_not(std::reverse_iterator<char const*>{end}, std::reverse_iterator<char const *>{beg},
					is_lws_char) + 1; // skip trailing linear whitespace
			m_cur_hdr.value.assign(beg, end);
			return n;
		}

		std::size_t v1x_request_line_parser::parse_line(char const *p, std::size_t n) {
			char const *beg = p, *end = p+n;
			auto sp1 = std::find(beg, end, ' ');
			auto sp2 = std::find(sp1+1, end, ' ');
			if (sp2 == end)
				return mark_bad(status_code_type::bad_request); // missing one or two space-character separators
			if (!is_method(beg, sp1))
				return mark_bad(status_code_type::bad_request); // invalid method
			std::error_code ec;
			auto u = uri::parse_uri_reference(sp1+1, sp2, ec);
			if (ec)
				return mark_bad(status_code_type::bad_request); // invalid URI
			if (!parse_http_version(sp2+1, end, m_ver))
				return mark_bad(status_code_type::bad_request); // invalid HTTP version
			m_method->assign(beg, sp1);
			m_uri->assign(std::move(u));
			m_uri->normalize_path();
			return mark_fin(n);
		}

		// Explicit template instantiations: These allow the parser templates to
		// be defined (in part) in this source file, instead of the header file,
		// to reduce compilation time.
		template class parser<v1x_line_parser<v1x_chunk_size_parser>>;
		template class parser<v1x_line_parser<v1x_empty_line_parser>>;
		template class parser<v1x_line_parser<v1x_headers_parser>>;
		template class parser<v1x_line_parser<v1x_request_line_parser>>;
	}
}

