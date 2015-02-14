// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// vim: set noet:

/** @file */

#include "clane_uri_parse.hpp"
#include <algorithm>
#include <cassert>
#include <cstring>

// FIXME: How to get GDB to support anonymous namespaces?
namespace {

		bool has_prefix(char const *beg, char const *end, char const *key) {
			assert(beg <= end);
			return static_cast<size_t>(end-beg) >= std::strlen(key) && !std::strncmp(beg, key, std::strlen(key));
		}

		bool is_unreserved_char(char c) {
			return std::isalpha(c) || std::isdigit(c) || c == '-' || c == '.' || c == '_' || c == '~';
		}

		bool is_sub_delim_char(char c) {
			return
				c == '!' ||
				c == '$' ||
				c == '&' ||
				c == '\'' ||
				c == '(' ||
				c == ')' ||
				c == '*' ||
				c == '+' ||
				c == ',' ||
				c == ';' ||
				c == '=';
		}

		bool is_pchar_char(char c) {
			return is_unreserved_char(c) || is_sub_delim_char(c) || c == ':' || c == '@';
		}

		bool is_scheme_nonfirst_char(char c) {
			// scheme = ALPHA *( ALPHA / DIGIT / "+" / "-" / "." )
			return std::isalnum(c) || c == '+' || c == '-' || c == '.';
		}

		bool is_ipvfut_address_char(char c) {
			return is_unreserved_char(c) || is_sub_delim_char(c) || c == ':';
		}

		static bool is_reg_name_char(char c) {
			return is_unreserved_char(c) || is_sub_delim_char(c);
		}

		bool is_port_char(char c) {
			return std::isdigit(c);
		}

		static bool is_user_info_char(char c) {
			// userinfo = *( unreserved / pct-encoded / sub-delims / ":" )
			return is_unreserved_char(c) || is_sub_delim_char(c) || c == ':';
		}

		bool is_path_char(char c) {
			return is_pchar_char(c) || c == '/';
		}

		bool is_query_char(char c) {
			return is_pchar_char(c) || c == '/' || c == '?';
		}

		bool is_fragment_char(char c) {
			return is_pchar_char(c) || c == '/' || c == '?';
		}

		size_t scan_h16(char const *beg, char const *end) {
			// h16 = 1*4HEXDIG
			char const *cur = beg;
			while (cur < end && cur < beg+4 && std::isxdigit(*cur))
				++cur;
			return cur-beg;
		}

}

namespace clane {
	namespace uri {

		bool is_fragment(char const *beg, char const *end) {
			return is_percent_encoded(beg, end, is_fragment_char);
		}

		bool is_ipv4_address(char const *beg, char const *end) {
			char const *cur = beg;
			int i = 0;
			while (true) {
				int val = 0, digs = 0;
				while (digs < 3 && std::isdigit(*cur)) {
					val *= 10;
					val += *cur - '0';
					++digs;
					++cur;
				}
				if (!digs || 255 < val)
					return false;
				++i;
				if (4 == i)
					return cur == end;
				if (*cur != '.')
					return false;
				++cur;
			}
		}

		bool is_ipv6_address(char const *beg, char const *end) {

			// IPv6address   =                            6( h16 ":" ) ls32
			//               /                       "::" 5( h16 ":" ) ls32
			//               / [               h16 ] "::" 4( h16 ":" ) ls32
			//               / [ *1( h16 ":" ) h16 ] "::" 3( h16 ":" ) ls32
			//               / [ *2( h16 ":" ) h16 ] "::" 2( h16 ":" ) ls32
			//               / [ *3( h16 ":" ) h16 ] "::"    h16 ":"   ls32
			//               / [ *4( h16 ":" ) h16 ] "::"              ls32
			//               / [ *5( h16 ":" ) h16 ] "::"              h16
			//               / [ *6( h16 ":" ) h16 ] "::"
			//   h16         = 1*4HEXDIG
			//	 ls32        = ( h16 ":" h16 ) / IPv4address */

			char const *cur = beg;
			int quad_cnt = 0;
			if (has_prefix(cur, end, "::")) {
				cur += 2;
			} else while (true) {
				if (quad_cnt == 6 && is_ls32(cur, end))
					return true;
				size_t stat = scan_h16(cur, end);
				if (!stat)
					return false;
				cur += stat;
				++quad_cnt;
				if (quad_cnt == 8)
					return cur == end;
				if (has_prefix(cur, end, "::")) {
					cur += 2;
					break;
				}
				if (cur == end || *cur != ':')
					return false;
				++cur;
			}

			// Invariant: The current position immediately follows a double colon,
			// either at the first byte following or at the end of the string.

			if (cur == end)
				return true;
			if (quad_cnt == 7)
				return false; // can't have more than seven quads with double colon
			while (true) {
				if (is_ls32(cur, end))
					return true;
				size_t stat = scan_h16(cur, end);
				if (!stat)
					return false;
				cur += stat;
				++quad_cnt;
				if (cur == end)
					return true;
				if (quad_cnt == 7 || *cur != ':')
					return false;
				++cur;
			}
		}

		bool is_ipvfut_address(char const *beg, char const *end) {
			// IPvFuture = "v" 1*HEXDIG "." 1*( unreserved / sub-delims / ":" )
			char const *cur = beg;
			if (cur == end || *cur != 'v')
				return false;
			++cur;
			char const *delim = std::find_if_not(cur, end, [](char c) { return std::isxdigit(c); });
			if (cur == delim)
				return false;
			cur = delim;
			if (cur == end || *cur != '.')
				return false;
			++cur;
			return cur < end && end == std::find_if_not(cur, end, is_ipvfut_address_char);
		}

		bool is_ls32(char const *beg, char const *end) {
			// ls32 = ( h16 ":" h16 ) / IPv4address
			if (is_ipv4_address(beg, end))
				return true;
			char const *cur = beg;
			size_t stat = scan_h16(cur, end);
			if (!stat)
				return false;
			cur += stat;
			if (cur == end || *cur != ':')
				return false;
			++cur;
			stat = scan_h16(cur, end);
			if (!stat)
				return false;
			cur += stat;
			return cur == end;
		}

		bool is_path(char const *beg, char const *end) {
			return is_percent_encoded(beg, end, is_path_char);
		}

		bool is_port(char const *beg, char const *end) {
			return end == std::find_if_not(beg, end, is_port_char);
		}

		bool is_query(char const *beg, char const *end) {
			return is_percent_encoded(beg, end, is_query_char);
		}

		bool is_reg_name(char const *beg, char const *end) {
			return is_percent_encoded(beg, end, is_reg_name_char);
		}

		bool is_scheme(char const *beg, char const *end) {
			// scheme = ALPHA *( ALPHA / DIGIT / "+" / "-" / "." )
			if (beg == end || !std::isalpha(*beg))
				return false;
			return end == std::find_if_not(beg+1, end, is_scheme_nonfirst_char);
		}

		bool is_user_info(char const *beg, char const *end) {
			// userinfo = *( unreserved / pct-encoded / sub-delims / ":" )
			return is_percent_encoded(beg, end, is_user_info_char);
		}

		std::string percent_decode(char const *beg, char const *end) {

			// Invariant: The input string is a valid percent-encoded string.
			assert(is_percent_encoded(beg, end, [](char c) { return true; }));

			// Pass 1. Determine how long the output string will be, and reserve
			// exactly that much space in the output string.
			size_t len = 0;
			char const *r = beg;
			while (r < end) {
				r += *r == '%' ? 3 : 1;
				++len;
			}

			std::string out;
			out.reserve(len);

			// Pass 2. Decode.
			r = beg;
			while (r < end) {
				if (*r == '%') {
					out.push_back(16*ascii::hexch_to_int(*(r+1)) + ascii::hexch_to_int(*(r+2)));
					r += 3;
				} else {
					out.push_back(*r);
					++r;
				}
			}
			return out;
		}

#if 0 // FIXME

		/** @brief Error category for URI errors */
		class error_category: public std::error_category {
		public:
			error_category() {}
			virtual char const *name() const noexcept { return "URI"; }
			virtual std::string message(int condition) const;
			virtual std::error_condition default_error_condition(int code) const noexcept;
		} const ecat;

		std::string error_category::message(int condition) const {
			switch (condition) {
				case static_cast<int>(error_code::invalid_scheme): return "invalid scheme";
				case static_cast<int>(error_code::invalid_user): return "invalid user";
				case static_cast<int>(error_code::invalid_host): return "invalid host";
				case static_cast<int>(error_code::invalid_port): return "invalid port";
				case static_cast<int>(error_code::invalid_path): return "invalid path";
				case static_cast<int>(error_code::invalid_query): return "invalid query";
				case static_cast<int>(error_code::invalid_fragment): return "invalid fragment";
			}
			return "unknown error";
		}

		std::error_condition error_category::default_error_condition(int code) const noexcept {
			switch (code) {
				case static_cast<int>(error_code::invalid_scheme):
				case static_cast<int>(error_code::invalid_user):
				case static_cast<int>(error_code::invalid_host):
				case static_cast<int>(error_code::invalid_port):
				case static_cast<int>(error_code::invalid_path):
				case static_cast<int>(error_code::invalid_query):
				case static_cast<int>(error_code::invalid_fragment):
					return std::error_condition(static_cast<int>(error_code::invalid_uri), ecat);
			}
			return std::error_condition(code, ecat);
		}

		static bool is_exact(char const *beg, char const *end, char const *s) {
			assert(beg <= end);
			size_t len = static_cast<size_t>(end - beg);
			return len == std::strlen(s) && !std::memcmp(beg, s, len);
		}

		static char const *const gen_delim = ":/?#[]@";

		uri parse_uri_reference(char const *beg, char const *end, std::error_code &e) {

			// URI-reference = URI / relative-ref
			//
			// URI           = scheme ":" hier-part [ "?" query ] [ "#" fragment ]
			//
			// hier-part     = "//" authority path-abempty
			//               / path-absolute
			//               / path-rootless
			//               / path-empty
			//
			// relative-ref  = relative-part [ "?" query ] [ "#" fragment ]
			//
			// relative-part = "//" authority path-abempty
			//               / path-absolute
			//               / path-noscheme
			//               / path-empty

			uri u;
			char const *r = beg; // current input position
			bool has_authority = false;

			std::string scheme, user, host, port, path, query, fragment;

			// optional scheme: If the string contains a general delimiter character
			// and the first such delimiter is a colon then this URI must contain a
			// scheme component.
			{
				char const *scheme_end = std::find_first_of(r, end, gen_delim, gen_delim+std::strlen(gen_delim));
				if (scheme_end < end && *scheme_end == ':') {
					if (!is_scheme(r, scheme_end)) {
						e.assign(static_cast<int>(error_code::invalid_scheme), ecat);
						return u;
					}
					scheme.assign(r, scheme_end);
					r = scheme_end + 1;
				}
			}

			// hierarchical or relative part: Because the scheme, if any, is
			// extracted, hierarchical and relative parts are now syntactically
			// equivalent. If the next two characters are "//" then the next component
			// is the authority.

			if (has_prefix(r, end, "//")) {

				r += 2;
				has_authority = true;

				// authority = [ userinfo "@" ] host [ ":" port ]

				// userinfo:
				char const *user_end = std::find_first_of(r, end, gen_delim, gen_delim+std::strlen(gen_delim));
				if (user_end < end && *user_end == '@') {
					if (!is_user_info(r, user_end)) {
						e.assign(static_cast<int>(error_code::invalid_user), ecat);
						return u;
					}
					user = percent_decode(r, user_end);
					r = user_end + 1;
				}

				// host = IP-literal / IPv4address / reg-name

				// host: The host ends with a colon (i.e., port), slash (i.e., path),
				// question mark (i.e., query), pound (i.e., fragment), or the end of
				// the string. However, the host may contain a colon if it's an IPv6
				// or IPvFuture address—but only if the address is delimited by matching
				// brackets ('[' and ']').

				char const *host_end;
				if (r < end && *r == '[') {
					host_end = std::find(r+1, end, ']');
					if (host_end == end || (!is_ipv6_address(r+1, host_end) && !is_ipvfut_address(r+1, host_end))) {
						e.assign(static_cast<int>(error_code::invalid_host), ecat);
						return u;
					}
					++host_end;
				} else {
					static char const *const host_delim = ":/?#";
					host_end = std::find_first_of(r, end, host_delim, host_delim+std::strlen(host_delim));
					if (!is_ipv4_address(r, host_end) && !is_reg_name(r, host_end)) {
						e.assign(static_cast<int>(error_code::invalid_host), ecat);
						return u;
					}
				}
				host = percent_decode(r, host_end);
				r = host_end;

				// port:
				if (r < end && *r == ':') {
					++r;
					static char const *const port_delim = "/?#";
					char const *port_end = std::find_first_of(r, end, port_delim, port_delim+std::strlen(port_delim));
					if (!is_port(r, port_end)) {
						e.assign(static_cast<int>(error_code::invalid_port), ecat);
						return u;
					}
					port.assign(r, port_end);
					r = port_end;
				}
			}

			// Invariant: The current position is at the first character of the path,
			// the '?' of the query, the '#' of the fragment, or at the end of the
			// string.

			if (r < end && *r != '?' && *r != '#') {
				if (has_authority && *r != '/') {
					e.assign(static_cast<int>(error_code::invalid_path), ecat); // with authority, nonempty path must be absolute
					return u;
				}
				static char const *const path_delim = "?#";
				char const *path_end = std::find_first_of(r, end, path_delim, path_delim+std::strlen(path_delim));
				if (!is_path(r, path_end)) {
					e.assign(static_cast<int>(error_code::invalid_path), ecat);
					return u;
				}
				path = percent_decode(r, path_end);
				r = path_end;
			}

			// Invariant: The current position is at the '?' of the query, the '#'
			// of the fragment, or at the end of the string.

			if (r < end && *r == '?') {
				++r;
				char const *query_end = std::find(r, end, '#');
				if (!is_query(r, query_end)) {
					e.assign(static_cast<int>(error_code::invalid_query), ecat);
					return u;
				}
				query = percent_decode(r, query_end);
				r = query_end;
			}

			// Invariant: The current position is at the '#' of the fragment or at the
			// end of the string.

			if (r < end) {
				assert(*r == '#');
				++r;
				char const *fragment_end = end;
				if (!is_fragment(r, fragment_end)) {
					e.assign(static_cast<int>(error_code::invalid_fragment), ecat);
					return u;
				}
				fragment = percent_decode(r, fragment_end);
				r = fragment_end;
			}

			// success:
			u.scheme = std::move(scheme);
			u.user = std::move(user);
			u.host = std::move(host);
			u.port = std::move(port);
			u.path = std::move(path);
			u.query = std::move(query);
			u.fragment = std::move(fragment);
			return u;
		}

		std::string uri::string() const {

			validate();

			// This algorithm is described in RFC 3986, §5.3 "Component
			// Recomposition".

			std::string out;
			if (!scheme.empty()) {
				out.append(scheme.data(), scheme.size());
				out.push_back(':');
			}

			if (has_authority())
				out.append("//");

			if (!user.empty()) {
				percent_encode(out, user, is_user_info_char);
				out.push_back('@');
			}

			if (!host.empty() && host[0] == '[') {
				out.append(host.data(), host.size()); // IP literal--no percent encoding
			} else {
				percent_encode(out, host, is_reg_name_char);
			}

			if (!port.empty()) {
				out.push_back(':');
				out.append(port.data(), port.size());
			}

			percent_encode(out, path, is_path_char);

			if (!query.empty()) {
				out.push_back('?');
				percent_encode(out, query, is_query_char);
			}

			if (!fragment.empty()) {
				out.push_back('#');
				percent_encode(out, fragment, is_fragment_char);
			}

			return out;
		}

		void uri::validate(std::error_code &e) const {

			// The following two restrictions are described in RFC 3986, §3 ("Syntax
			// Components"). They prevent composing a URI string that would yield,
			// when parsed, different component values.

			if (has_authority() && !path.empty() && path[0] != '/') {
				e.assign(static_cast<int>(error_code::invalid_path), ecat);
				return;
			}
			if (!has_authority() && has_prefix(path.data(), path.data()+path.size(), "//")) {
				e.assign(static_cast<int>(error_code::invalid_path), ecat);
				return;
			}

			// Furthermore, if the URI has no scheme and no authority and the path
			// does not begin with a slash then the first path segment must not
			// contain a colon.

			if (scheme.empty() && !has_authority() && !path.empty()) {
				static char const *const delim = ":/";
				char const *end = std::find_first_of(path.data(), path.data()+path.size(), delim, delim+std::strlen(delim));
				if (end != path.data()+path.size() && *end == ':') {
					e.assign(static_cast<int>(error_code::invalid_path), ecat);
					return;
				}
			}
		}

		void uri::normalize_path() {
			remove_dot_segments(path);
			remove_empty_segments(path);
		}

		void remove_dot_segments(std::string &s) {

			// This algorithm is described in RFC 3986, §5.2.4 ("Remove Dot
			// Segments").

			char *r = &s[0];
			char *w = &s[0];
			char *const rend = &s[0]+s.size();
			while (r < rend) {

				// A. If the input buffer begins with a prefix of "../" or "./", then
				// remove that prefix from the input buffer; otherwise,
				if (has_prefix(r, rend, "../")) {
					r += 3;
					continue;
				}
				if (has_prefix(r, rend, "./")) {
					r += 2;
					continue;
				}

				// B. if the input buffer begins with a prefix of "/./" or "/.", where
				// "." is a complete path segment, then replace that prefix with "/" in
				// the input buffer; otherwise,
				if (has_prefix(r, rend, "/./")) {
					r += 2;
					continue;
				}
				if (is_exact(r, rend, "/.")) {
					r += 1;
					*r = '/';
					continue;
				}

				// C. if the input buffer begins with a prefix of "/../" or "/..", where
				// ".." is a complete path segment, then replace that prefix with "/" in
				// the input buffer and remove the last segment and its preceding "/"
				// (if any) from the output buffer; otherwise,
				if (has_prefix(r, rend, "/../")) {
					r += 3;
					w = remove_last_path_segment(&s[0], w);
					continue;
				}
				if (is_exact(r, rend, "/..")) {
					r += 2;
					*r = '/';
					w = remove_last_path_segment(&s[0], w);
					continue;
				}

				// D. if the input buffer consists only of "." or "..", then remove that
				// from the input buffer; otherwise,
				if (is_exact(r, rend, ".")) {
					r += 1;
					continue;
				}
				if (is_exact(r, rend, "..")) {
					r += 2;
					continue;
				}

				// E. move the first path segment in the input buffer to the end of the
				// output buffer, including the initial "/" character (if any) and any
				// subsequent characters up to, but not including, the next "/"
				// character or the end of the input buffer.
				char *delim = std::find(*r == '/' ? r+1 : r, rend, '/');
				std::memmove(w, r, delim-r);
				w += delim-r;
				r = delim;
			}

			// truncate:
			s.resize(w-&s[0]);
		}

		void remove_empty_segments(std::string &s) {
			char *r = &s[0];
			char *w = &s[0];
			char *const rend = &s[0]+s.size();
			while (r < rend) {
				if (*r != '/' || r == &s[0] || *(r-1) != '/') {
					*w = *r;
					++w;
				}
				++r;
			}
			s.resize(w-&s[0]); // truncate
		}

		char *remove_last_path_segment(char *rbeg, char *rend) {
			assert(rbeg <= rend);
			char *slash = reinterpret_cast<char *>(memrchr(rbeg, '/', rend-rbeg));
			return slash ? slash : rbeg;
		}

#endif // #if 0

	}
}
