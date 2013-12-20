// vim: set noet:

/** @file */

#include "http_server.h"
#include <streambuf>

namespace clane {
	namespace http {

		class server_streambuf: public std::streambuf {
		private:
			//server_conn *conn;
		public:
			~server_streambuf() = default;
			server_streambuf(server_conn *conn);
			server_streambuf(server_streambuf const &) = delete;
			server_streambuf(server_streambuf &&) = default;
			server_streambuf &operator=(server_streambuf const &) = delete;
			server_streambuf &operator=(server_streambuf &&) = default;
		protected:
			virtual int sync();
			virtual int_type underflow();
			virtual int_type uflow();
			virtual int_type overflow(int_type c);
		};

		listener::mux_accept_result listener::accept() {
			mux_accept_result result{};
			auto accept_result = socket().accept();
			if (accept_result.aborted) {
				result.aborted = true;
			} else {
				result.conn.reset(new server_conn(std::move(accept_result.conn)));
			}
			return result;
		}

		void server_conn::acquire_ibuffer() {
			static size_t constexpr cap = 4096;
			ibuf_ref.reset(new char[cap]);
			// set underlying buffer attributes:
			ibuf = ibuf_ref.get();
			icap = cap;
			put_offset = 0;
		}

		void server_conn::finished() {
			// FIXME: Flush out data and mark connection for closure.
		}

		void server_conn::received(char *buf, size_t size) {

			// FIXME: Parse incoming data.
		}

		void server_conn::release_ibuffer() {
			ibuf_ref.reset();
		}

		void server_conn::reset() {
			// FIXME: Flush out data.
		}

		server_conn::server_conn(net::socket &&that_sock): net::mux_server_conn(std::move(that_sock)) {
		}

		void server_conn::timed_out() {
			mark_for_close();
		}

		server_streambuf::server_streambuf(server_conn *conn) { //: conn(conn) {
		}

		server_streambuf::int_type server_streambuf::overflow(int_type c) {
			// FIXME: implement
			return traits_type::eof();
		}

		int server_streambuf::sync() {
			// FIXME: implement
			return -1;
		}

		server_streambuf::int_type server_streambuf::uflow() {
			// FIXME: implement
			return traits_type::eof();
		}

		server_streambuf::int_type server_streambuf::underflow() {
			// FIXME: implement
			return traits_type::eof();
		}
	}
}

