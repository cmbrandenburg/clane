// vim: set noet:

#ifndef CLANE__HTTP_SERVER_HPP
#define CLANE__HTTP_SERVER_HPP

#include "clane_common.hpp"
#include "clane_event.hpp"
#include "clane_http_request.hpp"
#include "clane_socket.hpp"
#include "../include/clane_http.hpp"
#include <condition_variable>
#include <deque>
#include <mutex>
#include <string>
#include <thread>

namespace clane {
	namespace http {

		class oresponsestream: public std::ostream {
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
			std::chrono::steady_clock::time_point read_timeout;
			std::chrono::steady_clock::time_point write_timeout;
		public:
			~server() = default;
			server(): conn_cnt{}, max_header_size{8 * 1024},
			 	read_timeout{std::chrono::steady_clock::now() + std::chrono::seconds(120)},
				write_timeout{std::chrono::steady_clock::now() + std::chrono::seconds(120)} {}
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
