// vim: set noet:

/** @file */

#include "http_parse.h"
#include "http_server.h"

namespace clane {
	namespace http {

		class server_connection: public net::connection {
			std::shared_ptr<char> ibuf;
		public:
			virtual ~server_connection() noexcept {}
			server_connection(net::socket &&sock): net::connection(std::move(sock)) {}
			server_connection(server_connection &&) = default;
			server_connection &operator=(server_connection &&) = default;
		private:
			virtual void received(char *p, size_t n);
			virtual void finished();
			virtual void ialloc();
			virtual ready_result send_ready();
		};

		void server_connection::received(char *p, size_t n) {
			// TODO: implement
		}

		void server_connection::finished() {
			// TODO: implement
		}

		void server_connection::ialloc() {
			static constexpr size_t cap = 4096;
			ibuf.reset(new char[cap]);
			set_ibuf(ibuf.get(), cap);
		}

		server_connection::ready_result server_connection::send_ready() {
			// TODO: implement
			return ready_result::op_complete;
		}

		std::shared_ptr<net::signal> listener::new_connection(net::socket &&sock) {
			return std::make_shared<server_connection>(std::move(sock));
		}
	}
}

