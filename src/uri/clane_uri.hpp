// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// vim: set noet:

#ifndef CLANE__URI_HPP
#define CLANE__URI_HPP

/** @file */

#include "../ascii/clane_ascii.hpp"
#include "../clane_base.hpp"
#include "../../include/clane_uri.hpp"

namespace clane {
	namespace uri {

		/** @brief Parses a string as a URI
		 *
		 * @relatesalso uri
		 *
		 * @remark The parse_uri_reference() function parses a string as a URI. The
		 * @p beg and @p end arguments specify the range of the string.
		 *
		 * @return The parse_uri_reference() function returns a @ref uri instance
		 * for the string. If an error occurs while parsing or decoding then @p e is
		 * set to a nonzero value and the function returns an empty @ref uri
		 * instance. */
		uri parse_uri_reference(char const *beg, char const *end, std::error_code &e);

		/** @brief Parses a string as a URI
		 *
		 * @sa parse_uri_reference() */
		inline uri parse_uri_reference(std::string const &s, std::error_code &e) {
			return parse_uri_reference(&s[0], &s[0]+s.size(), e);
		}

		/** @brief Parses a string as a URI
		 *
		 * @sa parse_uri_reference() */
		inline uri parse_uri_reference(std::string const &s) {
			std::error_code e;
			uri out = parse_uri_reference(s, e);
			if (e)
				throw std::system_error(e);
			return out;
		}

		// Parser helpers:
		// The entire input string must match the rule.
		bool is_fragment(char const *beg, char const *end);
		bool is_ipv4_address(char const *beg, char const *end);
		bool is_ipv6_address(char const *beg, char const *end);
		bool is_ipvfut_address(char const *beg, char const *end);
		bool is_ls32(char const *beg, char const *end);
		bool is_path(char const *beg, char const *end);
		bool is_port(char const *beg, char const *end);
		bool is_query(char const *beg, char const *end);
		bool is_reg_name(char const *beg, char const *end);
		bool is_scheme(char const *beg, char const *end);
		bool is_user_info(char const *beg, char const *end);

		/** @brief Character test that always passes */
		inline bool is_any_char(char c) {
			return true;
		}

		/** @brief Returns whether a string is correctly percent-encoded and that
		 * all non-encoded characters pass a given test
		 *
		 * @tparam CharTest Function object with signature bool(char) */
		template <typename CharTest> bool is_percent_encoded(char const *beg, char const *end, CharTest &&test) {
			char const *i = beg;
			while (i < end) {
				if (*i == '%') {
					if (i+3 > end || !std::isxdigit(*(i+1)) || !std::isxdigit(*(i+2)))
						return false;
					i += 3;
				} else if (!test(*i)) {
					return false;
				} else {
					++i;
				}
			}
			return true;
		}

		/** @brief Decodes a percent-encoded string
		 *
		 * @remark The percent_decode() function decodes a percent-encoded string
		 * delimited by @p beg and @p end. The string must be a valid
		 * percent-encoded string.
		 *
		 * @return The percent_decode() function returns the decoded string. */
		inline std::string percent_decode(char const *beg, char const *end) {

			// Invariant: The input string is a valid percent-encoded string.
			assert(is_percent_encoded(beg, end, is_any_char));

			// Step 1. Determine how long the output string will be, and reserve
			// exactly that much space in the output string.
			size_t len = 0;
			char const *r = beg;
			while (r < end) {
				r += *r == '%' ? 3 : 1;
				++len;
			}

			std::string out;
			out.reserve(len);

			// Step 2. Decode.
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

		/** @brief Encodes a percent-encoded string and appends the result to a
		 * given string
		 *
		 * @remark This function encodes the string @p in and appends the
		 * percent-encoded output to @p out. Any character in the input string for
		 * which @p test returns false is percent-encoded. All other characters are
		 * appended verbatim. */
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

		/** @brief Removes dot segments ("." and "..") from a path string */
		void remove_dot_segments(std::string &s);

		/** @brief Removes all empty segments ("//") from a path string */
		void remove_empty_segments(std::string &s);

		/** @brief Removes the last segment from a path string */
		char *remove_last_path_segment(char *beg, char *end);

 	}
}

#endif // #ifndef CLANE__URI_HPP
