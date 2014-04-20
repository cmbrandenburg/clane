// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// vim: set noet:

#ifndef CLANE_HTTP_PREFIX_STRIPPER_HPP
#define CLANE_HTTP_PREFIX_STRIPPER_HPP

/** @file */

#include "clane_base_pub.hpp"
#include "clane_http_pub.hpp"
#include <functional>
#include <string>

namespace clane {
	namespace http {

		/** @brief @ref http_request_handling_page "HTTP request handler" that
		 * removes a beginning substring of a request's URI path before forwarding
		 * the request to another handler */
		template <typename Handler> class basic_prefix_stripper {
			std::string prefix;
			Handler h;
		public:
			~basic_prefix_stripper() {}
			template <typename Prefix>
				basic_prefix_stripper(Prefix &&p, Handler &&h): prefix(std::forward<Prefix>(p)), h(std::forward<Handler>(h)) {}
			basic_prefix_stripper(basic_prefix_stripper const &) = default;
			basic_prefix_stripper &operator=(basic_prefix_stripper const &) = default;
#ifndef CLANE_HAVE_NO_DEFAULT_MOVE
			basic_prefix_stripper(basic_prefix_stripper &&) = default;
			basic_prefix_stripper &operator=(basic_prefix_stripper &&) = default;
#else
			basic_prefix_stripper(basic_prefix_stripper &&that) noexcept;
			basic_prefix_stripper &operator=(basic_prefix_stripper &&that) noexcept;
#endif
			void swap(basic_prefix_stripper &that) noexcept;
			void operator()(response_ostream &rs, request &req);
		};

#ifdef CLANE_HAVE_NO_DEFAULT_MOVE

		template <typename Handler> basic_prefix_stripper<Handler>::basic_prefix_stripper(basic_prefix_stripper &&that) noexcept:
			prefix(std::move(that.prefix)),
			h(std::move(that.h)) {}

		template <typename Handler> basic_prefix_stripper<Handler> &
		basic_prefix_stripper<Handler>::operator=(basic_prefix_stripper &&that) noexcept {
			prefix = std::move(that.prefix);
			h = std::move(that.h);
			return *this;
		}

#endif

		template <typename Handler> void basic_prefix_stripper<Handler>::swap(basic_prefix_stripper &that) noexcept {
			std::swap(prefix, that.prefix);
			std::swap(h, that.h);
		}

		template <typename Handler> void basic_prefix_stripper<Handler>::operator()(response_ostream &rs, request &req) {
			if (req.uri.path.size() < prefix.size() ||
			    req.uri.path.substr(0, prefix.size()) != prefix) {
				rs.status = status_code::not_found;
				return;
			}
			req.uri.path.erase(0, prefix.size());
			h(rs, req);
		}

		/** @brief Specializes basic_prefix_stripper for a `std::function` request
		 * handler */
		typedef basic_prefix_stripper<std::function<void(response_ostream &, request &)>> prefix_stripper;

		/** @brief Constructs and returns a basic_prefix_stripper instance
		 *
		 * @relatesalso basic_prefix_stripper */
		template <typename Prefix, typename Handler> basic_prefix_stripper<Handler>
		make_prefix_stripper(Prefix &&p, Handler &&h) {
			basic_prefix_stripper<Handler> ps(std::forward<Prefix>(p), std::forward<Handler>(h));
			return ps;
		}

	}
}

#endif // #ifndef CLANE_HTTP_PREFIX_STRIPPER_HPP
