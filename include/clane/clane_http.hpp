// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// vim: set noet:

#ifndef CLANE_HTTP_HPP
#define CLANE_HTTP_HPP

/** @file */

#include "clane_ascii.hpp"
#include "clane_uri.hpp"
#include <boost/asio.hpp>
#include <memory>

namespace clane {
	namespace http {

		struct header_name_less {
			bool operator()(std::string const &a, std::string const &b) const {
				return ascii::icase_compare(a, b) < 0;
			}
		};

		/** @brief Map type for pairing HTTP header names to header values
		 *
		 * @remark A header_map is a `std::multimap` that pairs each header name to
		 * one or more header values.
		 *
		 * @remark Header names are case-insensitive; header values are
		 * case-sensitive. */
		typedef std::multimap<std::string, std::string, header_name_less> header_map;

		/** @brief HTTP header name–value pair
		 *
		 * @remark Header names are case-insensitive, and header values are case
		 * sensitive.
		 *
		 * @remark Headers are `typedef`'d as a `std::pair` owing to the use of a
		 * `std::multimap` for the header_map type. Consequently, the header name is
		 * stored as the pair's `first` member, and the header value is stored as
		 * the pair's `second` member. */
		typedef header_map::value_type header;

		inline bool header_equal(header const &a, header const &b) {
			return clane::ascii::icase_compare(a.first, b.first) == 0 && a.second == b.second;
		}

		inline bool header_less(header const &a, header const &b) {
			int n = clane::ascii::icase_compare(a.first, b.first);
			return n < 0 || (n == 0 && a.second < b.second);
		}

		inline bool header_less_equal(header const &a, header const &b) {
			int n = clane::ascii::icase_compare(a.first, b.first);
			return n < 0 || (n == 0 && a.second <= b.second);
		}

		inline bool operator==(header const &a, header const &b) { return header_equal(a, b); }
		inline bool operator!=(header const &a, header const &b) { return !header_equal(a, b); }
		inline bool operator<(header const &a, header const &b) { return header_less(a, b); }
		inline bool operator<=(header const &a, header const &b) { return header_less_equal(a, b); }
		inline bool operator>(header const &a, header const &b) { return !header_less_equal(a, b); }
		inline bool operator>=(header const &a, header const &b) { return !header_less(a, b); }

		inline bool operator==(header_map const &a, header_map const &b) {
			return a.size() == b.size() && std::equal(a.begin(), a.end(), b.begin(), header_equal);
		}
		inline bool operator!=(header_map const &a, header_map const &b) { return !(a == b); }
		inline bool operator<(header_map const &a, header_map const &b) {
			return std::lexicographical_compare(a.begin(), a.end(), b.begin(), b.end(), header_less);
		}
		inline bool operator<=(header_map const &a, header_map const &b) { return a == b || a < b; }
		inline bool operator>(header_map const &a, header_map const &b) { return !(a <= b); }
		inline bool operator>=(header_map const &a, header_map const &b) { return !(a < b); }

#if 0 // FIXME

		template <typename Handler> class basic_server {
			class impl;
			std::unique_ptr<impl> m_impl;

		public:

			basic_server(boost::asio::io_service &ios, Handler const &h):
				m_impl{std::make_unique<impl>(ios, h)}
			{}

			basic_server(boost::asio::io_service &ios, Handler &&h):
				m_impl{std::make_unique<impl>(ios, h)}
			{}

			template <typename ...HandlerArgs> basic_server(boost::asio::io_service &ios, HandlerArgs&&... args):
				m_impl{std::make_unique<impl>(ios, std::forward<HandlerArgs>(args)...)}
			{}

			basic_server(basic_server &&) = default;
			basic_server &operator=(basic_server &&) = default;

			void close() {
				m_impl->close();
			}

			/** Binds the server to an address
			 *
			 * @param addr Address to listen on. The address's format must be
			 * `<host>:<service>`, e.g., `":8080"` or `"localhost:http"`.
			 *
			 * @remark The add_listener() function binds the server to an address in
			 * addition to any addresses the server is already bound to. */
			void add_listener(std::string const &addr) {
				m_impl->add_listener(addr);
			}
		};

		template <typename Handler> basic_server<Handler> make_server(boost::asio::io_service &ios, Handler const &h) {
			return basic_server<Handler>(ios, h);
		}

		template <typename Handler> basic_server<Handler> make_server(boost::asio::io_service &ios, Handler &&h) {
			return basic_server<Handler>(ios, std::move(h));
		}

#endif // #if 0
	}
}

#include "clane_http.ipp"

#endif // #ifndef CLANE_HTTP_HPP
