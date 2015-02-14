// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// vim: set noet:

#ifndef CLANE_URI_PARSE_HPP
#define CLANE_URI_PARSE_HPP

/** @file */

#include "../ascii/clane_ascii.hpp"
#include <cctype>
#include <string>
#include <system_error>

namespace clane {
	namespace uri {

		/** URI parser error codes */
		enum class error_code {
			ok,
			invalid_uri,
			invalid_scheme,
			invalid_user,
			invalid_host,
			invalid_port,
			invalid_path,
			invalid_query,
			invalid_fragment
		};
	}
}

namespace std {
	template <> struct is_error_code_enum<clane::uri::error_code>: public true_type {};
}

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

		/** Uniform Resource Identifier
		 *
		 * @remark The @ref uri class encapsulates a URI as its individual
		 * components—e.g., scheme, path, etc. */
		struct uri {
			std::string scheme;
			std::string user;
			std::string host;
			std::string port;
			std::string path;
			std::string query;
			std::string fragment;

		public:

			/** Constructs this @ref uri as empty */
			uri() = default;

			/** Constructs this @ref uri from individual URI components */
			template <typename Source>
			uri(Source &&scheme, Source &&user, Source &&host, Source &&port, Source &&path, Source &&query, Source &&fragment):
				scheme(std::forward<Source>(scheme)),
				user(std::forward<Source>(user)),
				host(std::forward<Source>(host)),
				port(std::forward<Source>(port)),
				path(std::forward<Source>(path)),
				query(std::forward<Source>(query)),
				fragment(std::forward<Source>(fragment))
			{}

			/** Returns whether all URI components in this @ref uri are empty
			 * */
			bool empty() const {
				return
				 	scheme.empty() &&
				 	user.empty() &&
				 	host.empty() &&
				 	port.empty() &&
				 	path.empty() &&
				 	query.empty() &&
				 	fragment.empty();
			}

			/** Modifies this @ref uri to be empty */
			void clear() {
				scheme.clear();
				user.clear();
				host.clear();
				port.clear();
				path.clear();
				query.clear();
				fragment.clear();
			}

			/** Returns this @ref uri as a string
			 *
			 * @remark The string() function composes this @ref uri instance into a
			 * string. The resulting string is percent-encoded as needed.
			 *
			 * @remark Not all combinations of URI components are valid for
			 * composition. If this @ref uri instance is invalid then the string()
			 * function throws an exception. See the validate() function for more
			 * information about validity. */
			std::string string() const;

			/** Confirms whether this @ref uri instance may be composed into a
			 * string
			 *
			 * @remark Some combinations of URI components are invalid and cannot be
			 * used to compose a URI string. This occurs when such a composed string
			 * would yield, when parsed, a different set of URI components.
			 * Specifically, as per RFC 3986, §3 ("Syntax Components"): <blockquote>
			 * When authority is present, the path must either be empty or begin with
			 * a slash ("/") character.  When authority is not present, the path
			 * cannot begin with two slash characters ("//"). </blockquote>
			 *
			 * @remark Furthermore, when a URI has no scheme and no authority, and if
			 * the path is relative—i.e., does not begin with a slash—then the path's
			 * first segment must not contain a colon. For example,
			 * <code>"mailto:john_doe@example.com"</code> is an invalid path component
			 * without a scheme and authority because when composed and parsed, it
			 * would yield a scheme of <code>"mailto"</code> and a path of
			 * <code>"john_doe@example.com"</code>.
			 *
			 * @return If the URI is valid then the validate() function has no effect.
			 * Otherwise, it sets @p e to a nonzero value. */
			void validate(std::error_code &e) const;

			/** Confirms whether this @ref uri instance may be composed into a
			 * string
			 *
			 * @sa validate() */
			void validate() const {
				std::error_code e;
				validate(e);
				if (e)
					throw std::system_error(e);
			}

			/** Returns whether this @ref uri has an authority super-component
			 *
			 * @remark The has_authority() function returns whether this @ref uri
			 * instance has an authority—i.e., whether it has a nonempty user, host,
			 * or port component */
			bool has_authority() const {
				return !user.empty() || !host.empty() || !port.empty();
			}

			/** Modifies this @ref uri so as to remove dot segments ("." and
			 * "..") and empty segments (""). */
			void normalize_path();
		};

		/** Swaps one @ref uri instance with another */
		inline void swap(uri &a, uri &b) noexcept {
			swap(a.scheme, b.scheme);
			swap(a.user, b.user);
			swap(a.host, b.host);
			swap(a.port, b.port);
			swap(a.path, b.path);
			swap(a.query, b.query);
			swap(a.fragment, b.fragment);
		}

		/** Parses a string as a URI
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

		/** Parses a string as a URI
		 *
		 * @sa parse_uri_reference() */
		inline uri parse_uri_reference(std::string const &s, std::error_code &e) {
			return parse_uri_reference(&s[0], &s[0]+s.size(), e);
		}

		/** Parses a string as a URI
		 *
		 * @sa parse_uri_reference() */
		inline uri parse_uri_reference(std::string const &s) {
			std::error_code e;
			uri out = parse_uri_reference(s, e);
			if (e)
				throw std::system_error(e);
			return out;
		}
	}
}

#endif // #ifndef CLANE_URI_PARSE_HPP
