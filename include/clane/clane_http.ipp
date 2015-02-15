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
#include <mutex>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <system_error>

namespace clane {
	namespace http {

		class server_transaction: public std::iostream {
			friend class request_parser;

			class streambuf: public std::streambuf {};

		private:
			streambuf m_sbuf;

			unsigned    m_major_ver;
			unsigned    m_minor_ver;
			uri::uri    m_uri;
			std::string m_method;

		public:
			server_transaction():
				std::iostream{&m_sbuf}
			{}

		};

		/** Server-side parser engine */
		class request_parser {

			enum class state {
				request_line,
				header,
			} m_stat{state::request_line};

			std::string m_cur_line;
			uri::uri m_uri;

		public:
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
						if (std::string::npos == c)
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
	}
}


#endif // #ifndef CLANE_HTTP_IPP
