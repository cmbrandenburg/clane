// vim: set noet:

/** @file */

#ifndef CLANE_CLANE_HPP
#define CLANE_CLANE_HPP

#include <array>
#include <boost/asio.hpp>
#include <istream>
#include <list>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <system_error>

/** @brief Top-level project namespace */
namespace clane {

	/** @brief Hypertext Transfer Protocol */
	namespace http {

		class server_transaction: public std::iostream {
			friend class request_parser;

			class streambuf: public std::streambuf {};

		private:
			streambuf m_sbuf;

		public:
			server_transaction():
				std::iostream{&m_sbuf}
			{}

		};

		/** @brief Server-side parser engine */
		class request_parser {

		public:

			std::size_t parse(char const *p, std::size_t n, server_transaction &xact, std::error_code &ec);

		};

		template <typename Handler> class basic_server {

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

				/** @brief Number of bytes in the last buffer */
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

				template <typename ReadHandler> void start_async_receive(ReadHandler h) {
					socket.async_receive(boost::asio::buffer(m_ibufs.back().data()+m_ilen, ibuf_cap-m_ilen), h);
				}

				void consume(std::size_t n, std::error_code ec) {
					while (n) {
						auto c = m_pars.parse(m_ibufs.back().data()+m_ilen, n, *m_xacts.back(), ec);
						if (ec)
							return;
						m_ilen += c;
						n -= c;
						if (m_ilen == ibuf_cap) {
							assert(!n);
							m_ibufs.emplace_back();
							m_ilen = 0;
						}
						if (!c) {
							// The parses is done with the current transaction object. Make a
							// new one and continue parsing.
							new_transaction();
						}
					}

				}

			private:

				void new_transaction() {
					m_xacts.push_back(std::make_shared<server_transaction>());
				}

			};

		private:
			boost::asio::io_service &m_ios;
		public:
			Handler handler;
		private:
			std::list<boost::asio::ip::tcp::acceptor> m_accs;
			std::list<connection> m_conns;

		public:

			basic_server(boost::asio::io_service &ios, Handler const &h):
				m_ios(ios),
				handler{h}
			{}

			basic_server(boost::asio::io_service &ios, Handler &&h):
				m_ios(ios),
				handler{std::move(h)}
			{}

			template <typename ...HandlerArgs> basic_server(boost::asio::io_service &ios, HandlerArgs&&... args):
				m_ios(ios),
				handler{std::forward<HandlerArgs>(args)...}
			{}

			basic_server(basic_server &&) = default;
			basic_server &operator=(basic_server &&) = default;

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
				start_async_accept(m_accs.back());
			}

		private:

			void start_async_accept(boost::asio::ip::tcp::acceptor &acc) {
				m_conns.emplace_back(m_ios, handler);
				auto iter = end(m_conns); --iter;
				acc.async_accept(m_conns.back().socket, m_conns.back().peer_address,
						std::bind(&basic_server::on_accept_connection, this, std::placeholders::_1, std::ref(acc), iter));
			}

			void on_accept_connection(boost::system::error_code const &ec, boost::asio::ip::tcp::acceptor &acc,
					typename decltype(m_conns)::iterator conn_iter) {
				if (ec) {
					std::cerr << "Accept error: " << ec.message() << "\n";
					return;
				}
				auto &conn = *conn_iter;
				start_async_accept(acc);
				std::cerr << "Accepted connection: " << conn.peer_address.address().to_string() << ":" << conn.peer_address.port() << "\n";
				start_async_receive(conn_iter);
			}

			void start_async_receive(typename decltype(m_conns)::iterator conn_iter) {
				auto &conn = *conn_iter;
				conn.start_async_receive(
						std::bind(&basic_server::on_receive, this, std::placeholders::_1, std::placeholders::_2, conn_iter));
			}

			void on_receive(boost::system::error_code const &ec, std::size_t n, typename decltype(m_conns)::iterator conn_iter) {
				auto &conn = *conn_iter;

				if (ec) {

					if (boost::asio::error::get_misc_category() == ec.category() &&
					    boost::asio::error::misc_errors::eof == ec.value()) {
						// This is a graceful disconnection. Destruct the connection object
						// now. It has used shared pointers for any data that need to stay
						// around until after all outstanding request handlers have
						// returned.
						m_conns.erase(conn_iter);
						return;
					}

					// Otherwise, this is a harder error. Shut down the connection, same
					// as a graceful end-of-file error.
					std::cerr << "Receive error: " << ec.message() << ": Closing connection\n";
					m_conns.erase(conn_iter);
					return;
				}

				// TODO: Should we destruct the connection if this thread raises an
				// exception while consuming the incoming data?

				{
					std::error_code ec;
					conn.consume(n, ec);
				}

				// Continue the chain: start another receive operation in the
				// background.
				start_async_receive(conn_iter);
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

#endif // #ifndef CLANE_CLANE_HPP
