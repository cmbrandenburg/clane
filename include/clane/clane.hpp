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

/** @brief Top-level project namespace */
namespace clane {

	/** @brief Hypertext Transfer Protocol */
	namespace http {

		class server_transaction: public std::iostream {

			class streambuf: public std::streambuf {};

		private:
			streambuf m_sbuf;

		public:
			server_transaction():
				std::iostream{&m_sbuf}
			{}

		};

		template <typename Handler> class basic_server {

			class connection {
				static constexpr std::size_t ibuf_cap = 8192;
			public:
				boost::asio::ip::tcp::socket socket;
				boost::asio::ip::tcp::endpoint peer_address;
			private:
				Handler &m_handler;
				std::list<std::array<char, ibuf_cap>> m_ibufs;

				/** @brief Number of bytes in the last buffer */
				std::size_t m_ilen{};

				//request_parser

			public:

				connection(boost::asio::io_service &ios, Handler &h): socket{ios}, m_handler(h) {
					m_ibufs.emplace_back();
				}

				connection(connection &&) = default;
				connection &operator=(connection &&) = default;

				void start_async_read() {
					socket.async_receive(
						boost::asio::buffer(m_ibufs.back().data()+m_ilen, ibuf_cap-m_ilen),
						std::bind(&connection::on_receive, this, std::placeholders::_1, std::placeholders::_2));
				}

				void on_receive(boost::system::error_code const &e, std::size_t n) {

					std::cerr << "received: " << std::string(m_ibufs.back().data()+m_ilen, m_ibufs.back().data()+m_ilen+n) << '\n';

					// Continue the chain--start another receive operation in the
					// background.
					m_ilen += n;
					if (m_ilen == ibuf_cap) {
						m_ibufs.emplace_back();
						m_ilen = 0;
					}
					start_async_read();

				}

			private:

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
				acc.async_accept(m_conns.back().socket, m_conns.back().peer_address,
						std::bind(&basic_server::on_accept_connection, this, std::placeholders::_1, std::ref(acc), std::ref(m_conns.back())));
			}

			void on_accept_connection(boost::system::error_code const &e, boost::asio::ip::tcp::acceptor &acc, connection &conn) {
				start_async_accept(acc);
				std::cerr << "accepted connection: " << conn.peer_address.address().to_string() << ":" << conn.peer_address.port() << "\n";
				conn.start_async_read();
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
