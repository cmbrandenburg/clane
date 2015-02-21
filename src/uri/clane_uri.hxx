// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// vim: set noet:

#ifndef CLANE_URI_HXX
#define CLANE_URI_HXX

/** @file */

#include "ascii/clane_ascii.hxx"
#include "clane/clane_uri.hpp"
#include <cctype>

namespace clane {
	namespace uri {

		/** Returns whether a string is correctly percent-encoded and that all
		 * non-encoded characters pass a given test
		 *
		 * @tparam CharTest Function object with signature `bool(char)`. */
		template <typename CharTest> bool is_percent_encoded(char const *beg, char const *end, CharTest &&test) {
			char const *i = beg;
			while (i < end) {
				if (*i == '%') {
					if (i+3 > end || !std::isxdigit(*(i+1)) || !std::isxdigit(*(i+2)))
						return false;
					i += 3;
				} else {
					if (!test(*i))
						return false;
					++i;
				}
			}
			return true;
		}

		/** Returns whether a string is a valid URI fragment */
		bool is_fragment(char const *beg, char const *end);

		/** Returns whether a string is a valid URI port */
		bool is_port(char const *beg, char const *end);

		/** Returns whether a string is a valid URI scheme */
		bool is_scheme(char const *beg, char const *end);

		/** Returns whether a string is a valid URI IPv4 address */
		bool is_ipv4_address(char const *beg, char const *end);

		/** Returns whether a string is a valid URI IPv6 address */
		bool is_ipv6_address(char const *beg, char const *end);

		/** Returns whether a string is a valid URI IPvFuture address */
		bool is_ipvfut_address(char const *beg, char const *end);

		/** Returns whether a string is a valid final 32-bit component of a URI IPv6
		 * address */
		bool is_ls32(char const *beg, char const *end);

		/** Returns whether a string is a valid URI path */
		bool is_path(char const *beg, char const *end);

		/** Returns whether a string is a valid URI query */
		bool is_query(char const *beg, char const *end);

		/** Returns whether a string is a valid URI regular name */
		bool is_reg_name(char const *beg, char const *end);

		/** Returns whether a string is a valid URI user-info component */
		bool is_user_info(char const *beg, char const *end);

		/** Decodes a percent-encoded string
		 *
		 * @remark The percent_decode() function decodes a percent-encoded string
		 * delimited by @p beg and @p end. The string must be a valid
		 * percent-encoded string.
		 *
		 * @return The percent_decode() function returns the decoded string. */
		std::string percent_decode(char const *beg, char const *end);

		/** Percent-encodes string and appends the result to a given string
		 *
		 * @remark The percent_encode() function encodes the string @p in and
		 * appends the percent-encoded output to @p out. Any character in the input
		 * string for which @p test returns false is percent-encoded. All other
		 * characters are appended verbatim. */
		template <typename CharTest> void percent_encode(std::string &out, std::string const &in, CharTest &&test) {
			for (auto i = in.begin(); i != in.end(); ++i) {
				if (test(*i)) {
					out.push_back(*i);
				} else {
					out.push_back('%');
					out.push_back(ascii::int_to_hexch(*i / 16));
					out.push_back(ascii::int_to_hexch(*i % 16));
				}
			}
		}

		/** Removes dot segments ("." and "..") from a path string */
		void remove_dot_segments(std::string &s);

		/** Removes all empty segments ("//") from a path string */
		void remove_empty_segments(std::string &s);

		/** Removes the last segment from a URI path string */
		char *remove_last_path_segment(char *beg, char *end);
	}
}

#endif // #ifndef CLANE_URI_HXX
