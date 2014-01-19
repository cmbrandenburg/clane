// vim: set noet:

#include "uri.hpp"
#include <algorithm>
#include <cctype>
#include <cstring>
#include <functional>
#include <sstream>

namespace clane {
	namespace uri {

		// The syntax for URIs is specified in RFC3986.

		// simple character testers:
		static bool is_pchar(char c);
		static bool is_sub_delim(char c);
		static bool is_unreserved(char c);

		// complex character testers:
		static bool is_fragment_char(char c);
		static bool is_host_char(char c);
		static bool is_path_char(char c);
		static bool is_query_char(char c);
		static bool is_user_info_char(char c);

		// percent decoding:
		// May modify s even in case of error.
		static bool percent_decode(std::string &s, std::function<bool(char)> const &test_char);
		static void percent_encode(std::ostream &ostrm, std::string const &s, std::function<bool(char)> const &test_char);

		// validators and validating-decoders:
		// Decoders may modify s even in case of error.
		static bool decode_fragment(std::string &s);
		static bool decode_host(std::string &s);
		static bool decode_path(std::string &s);
		static bool decode_query(std::string &s);
		static bool decode_user_info(std::string &s);
		static bool is_ip_literal(std::string const &s);
		static bool is_ipv4_addr(std::string const &s);
		static bool is_ipv6_addr(std::string const &s);
		static bool is_ipvfut(std::string const &s);
		static bool is_port(std::string const &s);
		static bool is_scheme(std::string const &s);

		static void normalize_path(std::string &s);

		bool decode_fragment(std::string &s) {
			return percent_decode(s, is_fragment_char);
		}

		bool decode_host(std::string &s) {
			return is_ipv4_addr(s) || is_ip_literal(s) || percent_decode(s, is_host_char);
		}

		bool decode_path(std::string &s) {
			return percent_decode(s, is_path_char);
		}

		bool decode_query(std::string &s) {
			return percent_decode(s, is_query_char);
		}

		bool decode_user_info(std::string &s) {
			return percent_decode(s, is_user_info_char);
		}

		bool is_fragment_char(char c) {
			return is_pchar(c) || c == '/' || c == '?';
		}

		bool is_host_char(char c) {
			// host = IP-literal / IPv4address / reg-name
			// reg-name = *( unreserved / pct-encoded / sub-delims )
			return is_unreserved(c) || is_sub_delim(c);
		}

		bool is_ip_literal(std::string const &s) {
			// IP-literal = "[" ( IPv6address / IPvFuture  ) "]"
			// IPvFuture = "v" 1*HEXDIG "." 1*( unreserved / sub-delims / ":" )
			if (s.size() < 2 || s[0] != '[' || s[s.size()-1] != ']')
				return false;
			std::string subs = s.substr(1, s.size() - 2);
			return is_ipv6_addr(subs) || is_ipvfut(subs);
		}

		bool is_ipv4_addr(std::string const &s) {
			std::istringstream ss(s);
			ss.unsetf(std::ios_base::skipws);
			int octet;
			for (int i = 0; i < 3; ++i) {
				ss >> octet;
				if (!ss || octet < 0 || 255 < octet)
					return false;
				if (ss.get() != '.')
					return false;
			}
			ss >> octet;
			if (!ss || octet < 0 || 255 < octet || !ss.eof())
				return false;
			return true;
		}

		bool is_ipv6_addr(std::string const &s) {
			/* IPv6address   =                            6( h16 ":" ) ls32
			                 /                       "::" 5( h16 ":" ) ls32
			                 / [               h16 ] "::" 4( h16 ":" ) ls32
			                 / [ *1( h16 ":" ) h16 ] "::" 3( h16 ":" ) ls32
			                 / [ *2( h16 ":" ) h16 ] "::" 2( h16 ":" ) ls32
			                 / [ *3( h16 ":" ) h16 ] "::"    h16 ":"   ls32
			                 / [ *4( h16 ":" ) h16 ] "::"              ls32
			                 / [ *5( h16 ":" ) h16 ] "::"              h16
			                 / [ *6( h16 ":" ) h16 ] "::"
			   h16           = 1*4HEXDIG
				 ls32          = ( h16 ":" h16 ) / IPv4address */

			size_t pos = 0;
			auto extract_h16 = [&]() -> bool {
				if (pos == s.size() || !std::isxdigit(s[pos]))
					return false;
				++pos;
				for (int i = 1; i < 4 && pos < s.size() && std::isxdigit(s[pos]); ++i, ++pos);
				return true;
			};
			auto extract_ls32 = [&]() -> bool {
				if (is_ipv4_addr(s.substr(pos)))
					return true;
				if (!extract_h16())
					return false;
				if (pos == s.size() || s[pos] != ':')
					return false;
				++pos;
				return extract_h16();
			};

			int quad_cnt = 0;
			if (pos + 2 <= s.size() && s[pos] == ':' && s[pos+1] == ':') {
				pos += 2;
			} else {
				while (true) {
					if (quad_cnt == 6) {
						auto push_pos = pos;
						if (extract_ls32())
							return true;
						pos = push_pos;
					}
					if (!extract_h16())
						return false;
					++quad_cnt;
					if (quad_cnt == 8)
						return pos == s.size();
					if (pos + 2 <= s.size() && s[pos] == ':' && s[pos+1] == ':') {
						pos += 2;
						break;
					}
					if (pos + 1 <= s.size() && s[pos] == ':') {
						++pos;
						continue;
					}
					return false;
				}
			}

			// Invariant: The current position is immediately following a double
			// colon, either at the first byte following or at the end of the string.

			if (pos == s.size())
				return true;
			if (quad_cnt == 7)
				return false; // can have at most seven quads with double colon
			while (true) {
				auto push_pos = pos;
				if (quad_cnt < 6 && extract_ls32())
					return true;
				pos = push_pos;
				if (!extract_h16())
					return false;
				++quad_cnt;
				if (pos == s.size())
					return true;
				if (quad_cnt == 7 || s[pos] != ':')
					return false;
				++pos;
			}
		}

		bool is_ipvfut(std::string const &s) {
			auto rpos = s.begin();
			if (rpos == s.end() || *(rpos++) != 'v')
				return false;
			if (rpos == s.end() || !std::isxdigit(*(rpos++)))
				return false;
			while (rpos != s.end() && std::isxdigit(*rpos))
				++rpos;
			if (rpos == s.end() || *(rpos++) != '.')
				return false;
			if (rpos == s.end())
				return false;
			while (rpos != s.end() && (is_unreserved(*rpos) || is_sub_delim(*rpos) || *rpos == ':'))
				++rpos;
			return rpos == s.end();
		}

		bool is_path_char(char c) {
			return is_pchar(c) || c == '/';
		}

		bool is_pchar(char c) {
			return is_unreserved(c) || is_sub_delim(c) || c == ':' || c == '@';
		}

		bool is_port(std::string const &s) {
			return std::all_of(s.begin(), s.end(), isdigit);
		}

		bool is_query_char(char c) {
			return is_pchar(c) || c == '/' || c == '?';
		}

		bool is_scheme(std::string const &s) {
			// scheme = ALPHA *( ALPHA / DIGIT / "+" / "-" / "." )
			if (s.empty())
				return false;
			if (!std::isalpha(s[0]))
				return false;
			for (auto i = s.begin() + 1; i != s.end(); ++i) {
				if (!std::isalpha(*i) && !std::isdigit(*i) && *i != '+' && *i != '-' && *i != '.')
					return false;
			}
			return true;
		}

		bool is_sub_delim(char c) {
			return c == '!' || c == '$' || c == '&' || c == '\'' || c == '(' || c == ')' ||
				c == '*' || c == '+' || c == ',' || c == ';' || c == '=';
		}

		bool is_unreserved(char c) {
			return std::isalpha(c) || std::isdigit(c) || c == '-' || c == '.' || c == '_' || c == '~';
		}

		bool is_user_info_char(char c) {
			// userinfo = *( unreserved / pct-encoded / sub-delims / ":" )
			return is_unreserved(c) || is_sub_delim(c) || c == ':';
		}

		void normalize_path(std::string &s) {

			// There's an algorithm for removing dot segments that's described in RFC
			// 3986 §5.2.4 ("Remove Dot Segments"). This algorithm is similar, with
			// the difference that it keeps relative paths relative.

			bool const abs = !s.empty() && s[0] == '/';

			std::string::size_type rpos = 0, wpos = 0;
			auto remaining_size = [&]() -> std::string::size_type {
				return s.size() - rpos;
			};
			auto begins_with = [&](char const *key) -> bool {
				if (s.size() - rpos < strlen(key))
					return false;
				return 0 == memcmp(s.data() + rpos, key, strlen(key));
			};
			auto is = [&](char const *key) -> bool {
				return remaining_size() == strlen(key) && 0 == memcmp(s.data() + rpos, key, strlen(key));
			};
			auto remove_last_segment = [&]() {
				auto slash = wpos == 0 ? 0 : s.rfind('/', wpos - 1);
				wpos = slash == s.npos ? 0 : slash;
			};

			// 2. While the input buffer is not empty, loop…:
			while (remaining_size() > 0) {

				// A. If the input buffer begins with a prefix of "../" or "./", then
				// remove that prefix from the input buffer; …
				if (begins_with("../")) {
					rpos += 3;
					continue;
				}
				if (begins_with("./")) {
					rpos += 2;
					continue;
				}

				// B. If the input buffer begins with a prefix of "/./" or "/.", where
				// "." is a complete path segment, then replace that prefix with "/" in
				// the input buffer; …
				if (begins_with("/./")) {
					rpos += 2;
					continue;
				}
				if (is("/.")) {
					s[rpos += 1] = '/';
					continue;
				}

				// C. If the input buffer begins with a prefix of "/../" or "/..", where
				// ".." is a complete path segment, then replace that prefix with "/" in
				// the input buffer and remove the last segment and its preceding "/"
				// (if any) from the output buffer; …
				if (begins_with("/../")) {
					rpos += 3;
					remove_last_segment();
					continue;
				}
				if (is("/..")) {
					s[rpos += 2] = '/';
					remove_last_segment();
					continue;
				}

				// D. If the input buffer consists only of "." or "..", then remove that
				// from the input buffer; …
				if (is(".")) {
					rpos += 1;
					continue;
				}
				if (is("..")) {
					rpos += 2;
					continue;
				}

				// E. Move the first path segment in the input buffer to the end of the
				// output buffer, including the initial "/" character (if any) and any
				// subsequent characters up to, but not including, the next "/"
				// character or the end of the input buffer.
				//
				// Exception: If the output buffer is empty and the path is relative
				// then don't copy the initial "/" character. This keeps the output
				// buffer relative.
				auto slash = s.find('/', rpos);
				if (slash == rpos) {
					slash = s.find('/', rpos + 1);
					if (0 == wpos && !abs)
						++rpos; // exclude initial "/"
				}
				if (slash == s.npos)
					slash = s.size();
				if (wpos != rpos)
					std::copy(s.begin() + rpos, s.begin() + slash, s.begin() + wpos);
				wpos += slash - rpos;
				rpos = slash;
			}
			s.resize(wpos);
		}

		std::ostream &operator<<(std::ostream &ostrm, uri const &uri) {
			ostrm << uri.to_string();
			return ostrm;
		}

		bool parse_uri_reference(uri &uri, std::string const &s) {

			// special case: empty URI
			if (s.empty()) {
				uri.clear();
				return true;
			}

			clane::uri::uri t; // temporary, to protect URI argument in case of failure
			std::string::size_type cur = 0;

			// with scheme:
			if (s[0] != '/' && s[0] != '?' && s[0] != '#') {
				auto colon = s.find_first_of(":/", cur);
				if (colon != s.npos && s[colon] == ':') {
					t.scheme = s.substr(0, colon);
					if (!is_scheme(t.scheme))
						return false;
					cur = colon + 1;
				}
			}

			// with authority:
			if (s.size() >= cur + 2 && s[cur] == '/' && s[cur+1] == '/') {
				cur += 2;
				auto next = s.find_first_of("@:/?#", cur);

				// with user info:
				if (next != s.npos && s[next] == '@') {
					t.user_info = s.substr(cur, next - cur);
					if (!decode_user_info(t.user_info))
						return false;
					cur = next + 1;
					next = s.find_first_of(":/?#", cur);
				}

				// Invariant: The current position is at the first byte of the host.

				// Invariant: The next position is equal to npos or else at the ':' of
				// the port, the first byte of the path path, the '?' of the query, or
				// the '#' of the fragment.

				// extract host:
				t.host = s.substr(cur, next == s.npos ? next : next - cur);
				if (!decode_host(t.host))
					return false;
				if (next == s.npos) {
					swap(uri, t);
					return true;
				}
				cur = next;

				// with port:
				if (next != s.npos && s[next] == ':') {
					cur = next + 1;
					next = s.find_first_of("/?#", cur);
					t.port = s.substr(cur, next - cur);
					if (!is_port(t.port))
						return false;
					if (next == s.npos) {
						swap(uri, t);
						return true;
					}
					cur = next;
				}
			}

			// Invariant: The current position is at the first byte of the path, the
			// '?' of the query, or the '#' of the fragment;

			// extract path:
			if (s[cur] != '?' && s[cur] != '#') {
				auto next = s.find_first_of("?#", cur + 1);
				t.path = s.substr(cur, next == s.npos ? next : next - cur);
				if (!decode_path(t.path))
					return false;
				normalize_path(t.path);
				if (next == s.npos) {
					swap(uri, t);
					return true;
				}
				cur = next;
			}

			// Invariant: The current position is at the '?' of the query or the '#'
			// of the fragment.

			if (s[cur] == '?') {
				++cur;
				auto next = s.find('#', cur);
				t.query = s.substr(cur, next == s.npos ? next : next - cur);
				if (!decode_query(t.query))
					return false;
				if (next == s.npos) {
					swap(uri, t);
					return true;
				}
				cur = next + 1;
			}

			// Invariant: The current position is at the first byte of the fragment or
			// the '#' of the fragment.

			if (s[cur] == '#') {
				++cur;
			}
			t.fragment = s.substr(cur);
			if (!decode_fragment(t.fragment))
				return false;
			swap(uri, t);
			return true;
		}

		bool percent_decode(std::string &s, std::function<bool(char)> const &test_char) {
			auto from_hex = [](char c) -> int {
				if (std::isdigit(c))
					return c - '0';
				if ('a' <= c && c <= 'f')
					return c - 'a' + 10;
				return c - 'A' + 10;
			};
			std::string::size_type wpos = 0, rpos = 0;
			while (rpos < s.size()) {
				if (test_char(s[rpos])) {
					s[wpos] = s[rpos];
					++wpos;
					++rpos;
				} else if (s[rpos] == '%' && rpos + 2 < s.size() && std::isxdigit(s[rpos+1])  && std::isxdigit(s[rpos+2])) {
					s[wpos] = 0x10 * from_hex(s[rpos+1]) + from_hex(s[rpos+2]);
					++wpos;
					rpos += 3;
				} else {
					return false; // error: invalid character
				}
			}
			s.erase(wpos); // truncate string to only the decoded portion
			return true;
		}

		void percent_encode(std::ostream &ostrm, std::string const &s, std::function<bool(char)> const &test_char) {
			auto from_hex = [](int c) -> char {
				// assume 0 <= c < 16:
				if (c < 10)
					return '0' + c;
				return 'A' + (c - 10);
			};
			for (char i: s) {
				if (test_char(i)) {
					ostrm << i;
				} else {
					ostrm << '%' << from_hex(i >> 4) << from_hex(i & 0xf);
				}
			}
		}

		void swap(uri &a, uri &b) {
			using namespace std;
			swap(a.scheme, b.scheme);
			swap(a.user_info, b.user_info);
			swap(a.host, b.host);
			swap(a.port, b.port);
			swap(a.path, b.path);
			swap(a.query, b.query);
			swap(a.fragment, b.fragment);
		}

		void uri::clear() {
			scheme.clear();
			user_info.clear();
			host.clear();
			port.clear();
			path.clear();
			query.clear();
			fragment.clear();
		}

		bool uri::operator==(uri const &that) const {
			return
				scheme == that.scheme &&
				user_info == that.user_info &&
				host == that.host &&
				port == that.port &&
				path == that.path &&
				query == that.query &&
				fragment == that.fragment;
		}

		std::string uri::to_string() const {

			bool authority = !user_info.empty() || !host.empty() || !port.empty();

			// The syntax restrictions are from RFC 3986 §3 ("Syntax components").
			if (authority && !path.empty() && path[0] != '/')
				throw invalid_uri("nonempty authority with nonempty relative path");
			if (!authority && path.size() >= 2 && path[0] == '/' && path[1] == '/')
				throw invalid_uri("empty authority with double-slash path");

			// convert:
			std::ostringstream ss;
			if (!scheme.empty()) {
				ss << scheme << "://";
			} else if (authority) {
				ss << "//";
			}
			if (!user_info.empty()) {
				percent_encode(ss, user_info, is_user_info_char);
				ss << '@'; 
			}
			if (host.size() >= 2 && host[0] == '[' && host[host.size()-1] == ']') {
				ss << host; // IP literal--no percent encoding
			} else {
				percent_encode(ss, host, is_host_char);
			}
			if (!port.empty())
				ss << ':' << port;
			percent_encode(ss, path, is_path_char);
			if (!query.empty()) {
				ss << '?';
				percent_encode(ss, query, is_query_char);
			}
			if (!fragment.empty()) {
				ss << '#';
				percent_encode(ss, fragment, is_fragment_char);
			}
			return ss.str();
		}
	}
}

