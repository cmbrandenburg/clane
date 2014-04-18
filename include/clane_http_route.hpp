// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// vim: set noet:

#ifndef CLANE_HTTP_ROUTE_HPP
#define CLANE_HTTP_ROUTE_HPP

/** @file
 *
 * @brief HTTP request routing */

#include "clane_base_pub.hpp"
#include "clane_http_pub.hpp"
#include "clane_regex.hpp"
#include <list>

namespace clane {
	namespace http {

		/** @brief Matches HTTP requests against a set of criteria
		 *
		 * @remark The basic_route class pairs (1) a set of criteria with which
		 * an application may match incoming HTTP requests with (2) a request
		 * handler that the application calls to handle matched requests.
		 * Typically, an application uses an instance of basic_router to carry
		 * out the process of matching and dispatching requests. The
		 * basic_router class contains instances of basic_route.
		 *
		 * @remark A route may contain any of the following criteria:
		 *
		 * @remark
		 * - **Method.** The route method is a regular expression string. Any
		 *   matching request must have a status line _method_ field that
		 *   matches the route method regular expression.
		 * - **Path.** The route path is a regular expression string. Any
		 *   matching request must have a status line URI _path_ component that
		 *   matches the route path regular expression.
		 * - **Headers.** The route headers are a map of name–value pairs,
		 *   whereby each name is a literal string and each value is a regular
		 *   expression string. Any matching request must have, for each header
		 *   in the route, at least one header whose name literally matches the
		 *   route header's name (case-insensitive) and whose value matches the
		 *   route header's value as a regular expression.
		 *
		 * @remark When matching, a route logically <i>and</i>s its criteria and
		 * follows a <i>match-then-don't-match</i> strategy. An empty route—one
		 * that contains no criteria—matches all requests. Additional criteria
		 * only limit the requests that match. 
		 *
		 * @sa basic_router */
		template <typename Handler> class basic_route {
			typedef std::multimap<std::string, boost::regex> header_match_map;
			Handler h;
			boost::regex method_;
			boost::regex path_;
			header_match_map hdrs_;
		public:
			~basic_route() {}

			/** @brief */
			basic_route();
			basic_route(Handler &&h);
			basic_route(basic_route const &) = default;
			basic_route &operator=(basic_route const &) = default;
#ifndef CLANE_HAVE_NO_DEFAULT_MOVE
			basic_route(basic_route &&) = default;
			basic_route &operator=(basic_route &&) = default;
#else
			basic_route(basic_route &&that) noexcept;
			basic_route &operator=(basic_route &&that) noexcept;
#endif
			void swap(basic_route &that) noexcept;

			basic_route &handler(Handler &&h) { this->h = std::forward<Handler>(h); return *this; }

			// TODO: Replace the handle() method with handler() accessor
			// methods.
			void handle(response_ostream &rs, request &req) { h(rs, req); }
			bool match(request const &req) const;

			// criteria:
			template <typename Source> basic_route &method(Source s, regex::options_type reopts = regex::options::normal);
			template <typename Source> basic_route &host(Source s, regex::options_type reopts = regex::options::normal);
			template <typename Source> basic_route &path(Source s, regex::options_type reopts = regex::options::normal);
			template <typename Source1, typename Source2> basic_route &
			header(Source1 name, Source2 value, regex::options_type reopts = regex::options::normal);
		};

		template <typename Handler> basic_route<Handler>::basic_route():
			method_(""),
			path_("") {}

		template <typename Handler> basic_route<Handler>::basic_route(Handler &&h):
			h(std::forward<Handler>(h)),
			method_(""),
			path_("") {}

#ifdef CLANE_HAVE_NO_DEFAULT_MOVE

		template <typename Handler> basic_route<Handler>::basic_route(basic_route &&that) noexcept:
			h(std::move(that.h)),
		 	method_(std::move(that.method_)),
			path_(std::move(that.path_)),
			hdrs_(std::move(that.hdrs_)) {}

		template <typename Handler> basic_route<Handler> &
		basic_route<Handler>::operator=(basic_route &&that) noexcept {
			h = std::move(that.h);
			method_ = std::move(that.method_);
			path_ = std::move(that.path_);
			hdrs_ = std::move(that.hdrs_);
		 	return *this;
		}

#endif

		template <typename Handler> void basic_route<Handler>::swap(basic_route &that) noexcept {
			std::swap(h, that.h);
			std::swap(method_, that.method_);
			std::swap(path_, that.path_);
		}

		template <typename Handler> bool basic_route<Handler>::match(request const &req) const {

			// TODO: Should regular expression matching be "match" instead of
			// "search"?

			if (!boost::regex_search(req.method, method_) ||
			    !boost::regex_search(req.uri.path, path_))
				return false;

			// every header match item must match at least one header:
			// The name must match as normal--literally, case-insensitive. The value
			// matches as a regular expression.
			for (auto i = hdrs_.begin(); i != hdrs_.end(); ++i) {
				auto r = req.headers.equal_range(i->first);
				for (auto j = r.first; j != r.second; ++j) {
					if (boost::regex_search(j->second, i->second))
						goto next_header_match_item;
				}
				return false;
next_header_match_item:
				continue;
			}
			return true;
		}

		template <typename Handler> template <typename Source> basic_route<Handler> &
		basic_route<Handler>::method(Source s, regex::options_type reopts) {
			method_.assign(s, regex::boost_regex_option(reopts));
			return *this;
		}

		template <typename Handler> template <typename Source> basic_route<Handler> &
		basic_route<Handler>::host(Source s, regex::options_type reopts) {

			// remove any existing host items:
			auto r = hdrs_.equal_range("host");
			hdrs_.erase(r.first, r.second);

			// add new host item:
			header("host", std::forward<Source>(s), reopts);

			return *this;
		}

		template <typename Handler> template <typename Source> basic_route<Handler> &
		basic_route<Handler>::path(Source s, regex::options_type reopts) {
			path_.assign(s, regex::boost_regex_option(reopts));
			return *this;
		}

		template <typename Handler> template <typename Source1, typename Source2> basic_route<Handler> &
		basic_route<Handler>::header(Source1 name, Source2 value, regex::options_type reopts) {
#ifdef CLANE_HAVE_STD_MULTIMAP_EMPLACE
			hdrs_.emplace(name, boost::regex(value, regex::boost_regex_option(reopts)));
#else
			hdrs_.insert(header_match_map::value_type(name, boost::regex(value, regex::boost_regex_option(reopts))));
#endif
			return *this;
		}

		/** @brief Specializes basic_route for a `std::function` request handler */
		typedef basic_route<std::function<void(response_ostream &, request &)>> route;

		/** @brief Constructs and returns a basic_route instance
		 *
		 * @relatesalso basic_route */
		template <typename Handler> basic_route<Handler> make_route(Handler &&h) {
			return basic_route<Handler>(std::forward<Handler>(h));
		}

		/** @brief HTTP request handler that dispatches requests to one of many
		 * given request handlers according to specified criteria */
		template <typename Handler> class basic_router {
			std::list<std::unique_ptr<basic_route<Handler>>> routes;
		public:
			~basic_router() {}
			basic_router() {}
			basic_router(basic_router const &) = default;
			basic_router &operator=(basic_router const &) = default;
#ifndef CLANE_HAVE_NO_DEFAULT_MOVE
			basic_router(basic_router &&) = default;
			basic_router &operator=(basic_router &&) = default;
#else
			basic_router(basic_router &&that) noexcept { swap(that); }
			basic_router &operator=(basic_router &&that) noexcept { swap(that); return *this; }
#endif

			void swap(basic_router &that) noexcept;
			void operator()(response_ostream &rs, request &req);

			void clear() { routes.clear(); }

			basic_route<Handler> &new_route(Handler const &h);
			basic_route<Handler> &new_route(Handler &&h);
		};

		template <typename Handler> void basic_router<Handler>::swap(basic_router &that) noexcept {
			std::swap(routes, that.routes);
		}

		template <typename Handler> void basic_router<Handler>::operator()(response_ostream &rs, request &req) {

			// search for matching route:
			for (auto i = routes.begin(); i != routes.end(); ++i) {
				if ((*i)->match(req)) {
					(*i)->handle(rs, req);
					return;
				}
			}

			// no matching route:
			rs.status = status_code::not_found;
		}

		template <typename Handler> basic_route<Handler> &basic_router<Handler>::new_route(Handler const &h) {
			routes.push_back(std::unique_ptr<basic_route<Handler>>(new basic_route<Handler>(h)));
			return *routes.back().get();
		}

		template <typename Handler> basic_route<Handler> &basic_router<Handler>::new_route(Handler &&h) {
			routes.push_back(std::unique_ptr<basic_route<Handler>>(new basic_route<Handler>(std::move(h))));
			return *routes.back().get();
		}

		/** @brief Specializes basic_router for a `std::function` request handler */
		typedef basic_router<std::function<void(response_ostream &, request &)>> router;
	}
}

#endif // #ifndef CLANE_HTTP_ROUTE_HPP
