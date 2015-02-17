// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// vim: set noet:

#ifndef CLANE_HTTP_IPP
#define CLANE_HTTP_IPP

/** @file */

#include "clane_uri.hpp"
#include <array>
#include <boost/asio.hpp>
#include <istream>
#include <list>
#include <map>
#include <mutex>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <system_error>

namespace clane {

	/** Hypertext Transfer Protocol */
	namespace http {

		/** HTTP status codes, defined in RFC 2616 */
		enum class status_code {
			xcontinue                       = 100,
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

		/** Returns a human-readable name of an HTTP status code */
		char const *what(status_code c);

		/** Server-side object encapsulating both the incoming request and the
		 * outgoing response */
		class server_transaction {
			friend class server_engine;
			typedef uri::uri uri_type;

			class streambuf: public std::streambuf {};

		public:

			/** Request method */
			std::string   method;

			/** Request URI */
			uri::uri_type uri;

			/** Request major version—e.g., 1 in `"HTTP/1.0"` */
			unsigned      major_version;

			/** Request minor version—e.g., 0 in `"HTTP/1.0"` */
			unsigned      minor_version;

			/** Request message headers */
			header_map    request_headers;

			/** Response message headers */
			header_map    response_headers;

			/** Body of both the request and the response
			 *
			 * @remark Reading from the @ref body member effects reading from the
			 * request body. Writing to the @ref body member effects writing to the
			 * response body. */
			std::iostream body;


		private:
			streambuf m_sbuf;

		public:
			server_transaction():
				std::iostream{&m_sbuf}
			{}

		};

		class server_parser {
		};

#if 0 // FIXME
		/** Server-side parser engine */
		class request_parser {

			enum class state {
				request_line,
				header,
				body,
			} m_stat{state::request_line};

			std::string m_cur_line;
			uri::uri m_uri;

		public:
			static constexpr std::size_t nerror = -1;
			std::size_t parse(char const *p, std::size_t n, server_transaction &xact);
		};

		template <typename Handler> class basic_server<Handler>::impl {

			class connection {
				static constexpr std::size_t ibuf_cap = 8192;
			public:
				boost::asio::ip::tcp::socket socket;
				boost::asio::ip::tcp::endpoint peer_address;
			private:
				Handler &m_handler;
				request_parser m_pars;
				std::list<std::array<char, ibuf_cap>> m_ibufs;
				std::list<std::shared_ptr<server_transaction>> m_xacts;

				/** Number of bytes in the last buffer */
				std::size_t m_ilen{};

			public:

				connection(boost::asio::io_service &ios, Handler &h):
					socket{ios},
					m_handler(h)
				{
					m_ibufs.emplace_back();
					new_transaction();
				}

				connection(connection &&) = default;
				connection &operator=(connection &&) = default;

				void close() {
					socket.close();
				}

				template <typename ReadHandler> void start_async_receive(ReadHandler h) {
					socket.async_receive(boost::asio::buffer(m_ibufs.back().data()+m_ilen, ibuf_cap-m_ilen), h);
				}

				int consume(std::size_t n) {
					while (n) {
						auto c = m_pars.parse(m_ibufs.back().data()+m_ilen, n, *m_xacts.back());
						if (m_pars.nerror == c)
							return -1; // parser error
						if (!c) {
							// The parser is done with the current transaction object.
							m_handler(*m_xacts.back());
							new_transaction();
							continue;
						}
						m_ilen += c;
						n -= c;
						if (m_ilen == ibuf_cap) {
							// Ensure there's always free space to receive into.
							assert(!n);
							m_ibufs.emplace_back();
							m_ilen = 0;
						}
					}
					return 0; // no error
				}

			private:

				void new_transaction() {
					m_xacts.push_back(std::make_shared<server_transaction>());
				}

			};

		private:
			boost::asio::io_service &m_ios;
			Handler m_handler;
			std::list<boost::asio::ip::tcp::acceptor> m_accs;
			std::list<connection> m_conns;

		public:

			impl(boost::asio::io_service &ios, Handler const &h):
				m_ios(ios),
				m_handler{h}
			{}

			impl(boost::asio::io_service &ios, Handler &&h):
				m_ios(ios),
				m_handler{std::move(h)}
			{}

			template <typename ...HandlerArgs> impl(boost::asio::io_service &ios, HandlerArgs&&... args):
				m_ios(ios),
				m_handler{std::forward<HandlerArgs>(args)...}
			{}

			impl(impl &&) = default;
			impl &operator=(impl &&) = default;

			void close() {
				std::for_each(begin(m_accs), end(m_accs), [](boost::asio::ip::tcp::acceptor &acc) {
					acc.close();
				});
				std::for_each(begin(m_conns), end(m_conns), [](connection &conn) {
					conn.close();
				});
			}

			void add_listener(std::string const &addr) {

				// Split the address argument into its host and service parts.
				auto sep = addr.find(':');
				if (sep == addr.npos) {
					std::ostringstream ess;
					ess << "Invalid listener address '" << addr << "': Address must be of form '<host>:<service>'";
					throw std::invalid_argument(ess.str());
				}
				auto host = addr.substr(0, sep);
				auto service = addr.substr(sep+1);

				// Create a new acceptor and begin the first asynchronous accept
				// operation.
				boost::asio::ip::tcp::resolver resolv{m_ios};
				boost::asio::ip::tcp::resolver::query q{host, service};
				m_accs.emplace_back(m_ios, resolv.resolve(q)->endpoint()); // blocking resolver lookup
				m_accs.back().listen();
				start_async_accept(--end(m_accs));
			}

		private:

			void start_async_accept(typename decltype(m_accs)::iterator acc_iter) {
				boost::asio::ip::tcp::acceptor &acc = *acc_iter;
				m_conns.emplace_back(m_ios, m_handler);
				auto conn_iter = end(m_conns); --conn_iter;
				acc.async_accept(m_conns.back().socket, m_conns.back().peer_address,
						std::bind(&impl::on_accept_connection, this, std::placeholders::_1, acc_iter, conn_iter));
			}

			void on_accept_connection(boost::system::error_code const &ec, typename decltype(m_accs)::iterator acc_iter,
					typename decltype(m_conns)::iterator conn_iter) {
				boost::asio::ip::tcp::acceptor &acc = *acc_iter;
				if (ec) {
					if (boost::asio::error::get_system_category() == ec.category() &&
							boost::asio::error::basic_errors::operation_aborted != ec.value()) {
						m_accs.erase(acc_iter);
						return;
					}
					std::ostringstream ess;
					ess << "Accept error: " << ec.message();
					throw std::runtime_error(ess.str());
				}
				auto &conn = *conn_iter;
				start_async_accept(acc_iter);
				std::cerr << "Accepted connection: " << conn.peer_address.address().to_string() << ":" << conn.peer_address.port() << "\n";
				start_async_receive(conn_iter);
			}

			void start_async_receive(typename decltype(m_conns)::iterator conn_iter) {
				auto &conn = *conn_iter;
				conn.start_async_receive(
						std::bind(&impl::on_receive, this, std::placeholders::_1, std::placeholders::_2, conn_iter));
			}

			void on_receive(boost::system::error_code const &ec, std::size_t n, typename decltype(m_conns)::iterator conn_iter) {
				auto &conn = *conn_iter;

				if (ec) {

					if (boost::asio::error::get_misc_category() == ec.category() &&
					    boost::asio::error::misc_errors::eof == ec.value()) {
						// This is a graceful disconnection. Destruct the connection object
						// now. It uses shared pointers for any data that need to persist
						// until after all outstanding request handlers have returned.
						m_conns.erase(conn_iter);
						return;
					}

					if (boost::asio::error::get_system_category() == ec.category() &&
					    boost::asio::error::basic_errors::operation_aborted == ec.value()) {
						// Same as a graceful disconnection.
						m_conns.erase(conn_iter);
						return;
					}

					// Otherwise, this is a hard error. Shut down the connection.
					m_conns.erase(conn_iter);
					std::ostringstream ess;
					ess << "Receive error: " << ec.message() << ": Closing connection\n";
					throw std::runtime_error(ess.str());
				}

				// TODO: Should we destruct the connection if this thread raises an
				// exception while consuming the incoming data?

				{
					std::error_code ec;
					if (0 > conn.consume(n)) {
						m_conns.erase(conn_iter);
						return;
					}
				}

				// Continue the chain: start another receive operation in the
				// background.
				start_async_receive(conn_iter);
			}
		};

#endif // #if 0
	}
}


#endif // #ifndef CLANE_HTTP_IPP
