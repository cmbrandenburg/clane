// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// vim: set noet:

/** @file */

#include "ascii/clane_ascii_impl.hpp"
#include "clane/clane_http.hpp"
#include "clane_http_parse.hpp"

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
		return end == std::find_if_not(beg, end, is_token_char);
	}
}

namespace clane {
	namespace http {

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

	}
}

