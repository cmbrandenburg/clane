// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// vim: set noet:

#ifndef CLANE_URI_PUB_HPP
#define CLANE_URI_PUB_HPP

/** @file
 *
 * @brief Uniform Resource Identifier */

#include "clane_base_pub.hpp"
#include <system_error>

namespace clane {

	namespace uri {
	
		/** @brief Uniform Resource Identifier
		 *
		 * @remark The @ref uri class encapsulates a URI as its individual
		 * components—e.g., scheme, path, etc. */
		class uri {

		public:

			std::string scheme;
			std::string user;
			std::string host;
			std::string port;
			std::string path;
			std::string query;
			std::string fragment;

		public:

			/** @brief Destructs this @ref uri */
			~uri() {}

			/** @brief Constructs this @ref uri as empty */
			uri() {}

			// FIXME: add parser constructors here

			/** @brief Constructs this @ref uri from individual URI components */
			template <typename Source>
			uri(Source &&scheme, Source &&user, Source &&host, Source &&port, Source &&path, Source &&query, Source &&fragment):
				scheme(std::forward(scheme)),
				user(std::forward(user)),
				host(std::forward(host)),
				port(std::forward(port)),
				path(std::forward(path)),
				query(std::forward(query)),
				fragment(std::forward(fragment)) {}

			/** @brief Constructs this @ref uri as a copy of another */
			uri(uri const &that):
				scheme(that.scheme),
				user(that.user),
				host(that.host),
				port(that.port),
				path(that.path),
				query(that.query),
				fragment(that.fragment) {}

			/** @brief Constructs this @ref uri as a move of another */
			uri(uri &&that):
				scheme(std::move(that.scheme)),
				user(std::move(that.user)),
				host(std::move(that.host)),
				port(std::move(that.port)),
				path(std::move(that.path)),
				query(std::move(that.query)),
				fragment(std::move(that.fragment)) {}

			/** @brief Assigns this @ref uri as a copy of another */
			uri &operator=(uri const &that) {
				scheme = that.scheme;
				user = that.user;
				host = that.host;
				port = that.port;
				path = that.path;
				query = that.query;
				fragment = that.fragment;
				return *this;
			}

			/** @brief Assigns this @ref uri as a move of another */
			uri &operator=(uri &&that) {
				scheme = std::move(that.scheme);
				user = std::move(that.user);
				host = std::move(that.host);
				port = std::move(that.port);
				path = std::move(that.path);
				query = std::move(that.query);
				fragment = std::move(that.fragment);
				return *this;
			}

			/** @brief Swaps this @ref uri with another */
			void swap(uri &that) noexcept {
				std::swap(scheme, that.scheme);
				std::swap(user, that.user);
				std::swap(host, that.host);
				std::swap(port, that.port);
				std::swap(path, that.path);
				std::swap(query, that.query);
				std::swap(fragment, that.fragment);
			}

			/** @brief Returns whether all URI components in this @ref uri are empty
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

			/** @brief Modifies this @ref uri to be empty */
			void clear() {
				scheme.clear();
				user.clear();
				host.clear();
				port.clear();
				path.clear();
				query.clear();
				fragment.clear();
			}

			/** @brief Modifies this @ref uri so as to remove dot segments ("." and
			 * "..") and empty segments (""). */
			void normalize_path(); // FIXME

			/** @brief Returns this @ref uri as a string
			 *
			 * @remark The string() function composes this @ref uri instance into a
			 * string. The resulting string is percent-encoded as needed.
			 *
			 * @remark Not all combinations of URI components are valid for
			 * composition. If this @ref uri instance is invalid then the string()
			 * function throws an @ref error exception. See the validate() function
			 * for more information about validity. */
			std::string string() const;

			/** @brief Confirms whether this @ref uri instance may be composed into a
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

			/** @brief Confirms whether this @ref uri instance may be composed into a
			 * string
			 *
			 * @sa validate() */
			void validate() const {
				std::error_code e;
				validate(e);
				if (e)
					throw std::system_error(e);
			}

			/** @brief Returns whether this @ref uri has an authority super-component
			 *
			 * @remark The has_authority() function returns whether this @ref uri
			 * instance has an authority—i.e., whether it has a user, host, and/or
			 * port component */
			bool has_authority() const {
				return !user.empty() || !host.empty() || !port.empty();
			}
		};

	}
}

#endif // #ifndef CLANE_URI_PUB_HPP
