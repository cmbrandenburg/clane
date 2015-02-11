// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// vim: set noet:

/** @file */

#ifndef CLANE_HTTP_HPP
#define CLANE_HTTP_HPP

#include "clane_uri.hpp"
#include <boost/asio.hpp>
#include <memory>

namespace clane {

	/** @brief Hypertext Transfer Protocol */
	namespace http {

		/** @brief HTTP status codes, defined in RFC 2616 */
		enum class status_code {
			cont                            = 100,
			switching_protocols             = 101,
			ok                              = 200,
			created                         = 201,
			accepted                        = 202,
			non_authoritative_information   = 203,
			no_content                      = 204,
			reset_content                   = 205,
			partial_content                 = 206,
			multiple_choices                = 300,
			moved_permanently               = 301,
			found                           = 302,
			see_other                       = 303,
			not_modified                    = 304,
			use_proxy                       = 305,
			temporary_redirect              = 307,
			bad_request                     = 400,
			unauthorized                    = 401,
			payment_required                = 402,
			forbidden                       = 403,
			not_found                       = 404,
			method_not_allowed              = 405,
			not_acceptable                  = 406,
			proxy_authentication_required   = 407,
			request_timeout                 = 408,
			conflict                        = 409,
			gone                            = 410,
			length_required                 = 411,
			precondition_failed             = 412,
			request_entity_too_long         = 413,
			request_uri_too_long            = 414,
			unsupported_media_type          = 415,
			requested_range_not_satisfiable = 416,
			expectation_failed              = 417,
			internal_server_error           = 500,
			not_implemented                 = 501,
			bad_gateway                     = 502,
			service_unavailable             = 503,
			gateway_timeout                 = 504,
			http_version_not_supported      = 505,
		};

		/** @brief Returns a human-readable name of an HTTP status code */
		char const *what(status_code c);

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

			/** @brief Binds the server to an address
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
	}
}

#include "clane_http.ipp"

#endif // #ifndef CLANE_HTTP_HPP
