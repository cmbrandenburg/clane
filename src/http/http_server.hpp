// vim: set noet:

#ifndef CLANE__HTTP_SERVER_HPP
#define CLANE__HTTP_SERVER_HPP

#include "http_common.hpp"
#include "http_message.hpp"
#include "http_status.hpp"
#include "../net/net_event.hpp"
#include "../net/net_socket.hpp"
#include <condition_variable>
#include <deque>
#include <mutex>
#include <string>
#include <thread>

namespace clane {
	namespace http {

		class oresponsestream: public std::ostream {
		public:
			status_code &status;
			header_map &headers;
		public:
			virtual ~oresponsestream() = default;
			explicit oresponsestream(std::streambuf *sb, status_code &stat_code, header_map &hdrs): std::ostream{sb},
					status(stat_code), headers(hdrs) {}
			oresponsestream(oresponsestream const &) = delete;
			oresponsestream(oresponsestream &&) = default;
			oresponsestream &operator=(oresponsestream const &) = delete;
			oresponsestream &operator=(oresponsestream &&) = default;
		};

		typedef std::function<void(oresponsestream &, request &)> handler;

		class server {

			class scoped_conn_ref {
				server *ser;
			public:
				~scoped_conn_ref();
				scoped_conn_ref(server *ser);
				scoped_conn_ref(scoped_conn_ref const &) = delete;
				scoped_conn_ref(scoped_conn_ref &&that) noexcept: ser{} { swap(that); }
				scoped_conn_ref &operator=(scoped_conn_ref const &) = delete;
				scoped_conn_ref &operator=(scoped_conn_ref &&that) noexcept { swap(that); return *this; }
				void swap(scoped_conn_ref &that) noexcept { std::swap(ser, that.ser); }
			};

			std::deque<net::socket> listeners;
			net::event term_event;
			std::deque<std::thread> thrds;
			int conn_cnt;
			std::mutex conn_cnt_mutex;
			std::condition_variable conn_cnt_cond;

		public:
			handler root_handler;
			size_t max_header_size;
			std::chrono::steady_clock::duration read_timeout;
			std::chrono::steady_clock::duration write_timeout;
		public:
			~server() = default;
			server(): conn_cnt{}, max_header_size{8 * 1024} {}
			server(server const &) = delete;
			server(server &&) = delete;
			server &operator=(server &&) = delete;
			void add_listener(char const *addr);
			void add_listener(std::string const &addr);
			void run();
			void terminate();
		private:
			void listen_main(net::socket lis);
			void connection_main(net::socket conn, scoped_conn_ref conn_ref);
		};
	}
}

#endif // #ifndef CLANE__HTTP_SERVER_HPP
