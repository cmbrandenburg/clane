// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// vim: set noet:

#ifndef CLANE_HTTP_SERVER_HPP
#define CLANE_HTTP_SERVER_HPP

/** @file */

#include "clane_uri.hpp"
#include <boost/asio.hpp>
#include <memory>

namespace clane {
	namespace http {

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

#include "clane_http_server.ipp"

#endif // #ifndef CLANE_HTTP_SERVER_HPP
