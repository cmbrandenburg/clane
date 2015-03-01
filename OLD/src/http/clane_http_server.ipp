// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// vim: set noet:

#ifndef CLANE_HTTP_IPP
#define CLANE_HTTP_IPP

/** @file */

#include "clane_http_message.hpp"
#include "uri/clane_uri.hpp"
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
	namespace http {

		/** Server-side object encapsulating both the incoming request and the
		 * outgoing response */
		class server_transaction {
			friend class server_engine;
			typedef uri::uri uri_type;

			class streambuf: public std::streambuf {};

			streambuf m_sbuf;

		public:

			/** Request method */
			std::string method;

			/** Request URI */
			uri_type    uri;

			/** Request major version—e.g., 1 in `"HTTP/1.0"` */
			unsigned    major_version;

			/** Request minor version—e.g., 0 in `"HTTP/1.0"` */
			unsigned    minor_version;

			/** Request message headers */
			header_map  request_headers;

			/** Request message footers */
			header_map  request_footers;

			/** Response message headers */
			header_map  response_headers;

			/** Body of both the request and the response
			 *
			 * @remark Reading from the @ref body member effects reading from the
			 * request body. Writing to the @ref body member effects writing to the
			 * response body. */
			std::iostream body;

		public:
			server_transaction():
				body{&m_sbuf}
			{}

		private:
			void more_body(char const *p, size_t n);
		};

		/** Server-side message-parsing engine.
		 *
		 * The server_engine drives all server-side request handling. It takes
		 * as input all data received on a connection and emits as output state
		 * changes needed for handling any incoming requests on the connection.
		 * */
		class server_engine {
			typedef http::status_code status_code_type;
			class impl;
		public:
			struct state {

				/** The server should close the connection */
				static constexpr int close = 1<<0;

				/** The server should send a response without invoking the request
				 * handler */
				static constexpr int no_handler = 1<<1;

				/** The server should invoke the request handler */
				static constexpr int body_init = 1<<2;

				/** The server should terminate the request body stream */
				static constexpr int body_eof = 1<<3;

				/** The server should handle the request body content */
				static constexpr int body_more = 1<<4;

				/** The server should generate an I/O error for incoming body content */
				static constexpr int body_error = 1<<5;
			};

		private:
			std::unique_ptr<impl> m_impl;

		public:

			server_engine(server_transaction *xact);

			/** Reuse parser to begin parsing a new request */
			void reset(server_transaction *xact);

			/** Returns the state flags
			 *
			 * @remark The state flags denote the latest result of parsing the
			 * incoming connection data. */
			int state() const;

			/** Set all state flags to zero */
			void clear_state();

			/** Returns the HTTP status code in case of error */
			status_code_type status_code();

			/** Parse some incoming connection data */
			std::size_t parse_some(char const *p, std::size_t n);

		};

#if 0 // FIXME
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