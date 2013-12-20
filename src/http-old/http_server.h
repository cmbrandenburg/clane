// vim: set noet:

#ifndef CLANE_HTTP_SERVER_H
#define CLANE_HTTP_SERVER_H

/** @file
 *
 * @brief Server-side HTTP */

#include "http_common.h"
#include "http_parse.h"

namespace clane {
	namespace http {

		class server_streambuf;

		class server_conn: public net::mux_server_conn {
			friend class server_streambuf;
			std::shared_ptr<char> ibuf_ref;
		public:
			~server_conn() = default;
			server_conn(net::socket &&that_sock);
			server_conn(server_conn const &) = delete;
			server_conn(server_conn &&) = default;
			server_conn &operator=(server_conn const &) = delete;
			server_conn &operator=(server_conn &&) = default;
		protected:
			virtual void received(char *buf, size_t size);
			virtual void finished();
			virtual void reset();
			virtual void timed_out();
			virtual void acquire_ibuffer();
			virtual void release_ibuffer();
		};

		class listener: public net::mux_listener {
		public:
			~listener() = default;
			listener(net::socket &&that_sock);
			listener(listener const &) = delete;
			listener(listener &&) = default;
			listener &operator=(listener const &) = delete;
			listener &operator=(listener &&) = default;
		protected:
			virtual mux_accept_result accept();
		};
	}
}

#endif // #ifndef CLANE_HTTP_SERVER_H
