// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// vim: set noet:

#ifndef CLANE_HTTP_SERVER_HPP
#define CLANE_HTTP_SERVER_HPP

/** @file */

#include "clane_base.hpp"
#include "clane_http_message.hpp"
#include "clane_net.hpp"
#include "clane_sync.hpp"
#include <deque>
#include <istream>
#include <memory>
#include <thread>

namespace clane {

	namespace http {

		/** @brief Server-side class for writing an HTTP response message */
		class response_ostream: public std::ostream {
		public:

			/** @brief Status code to send
			 *
			 * @remark Applications may set the @ref status member to send an
			 * HTTP response message with an http::status_code status code other than
			 * the default value of `200 OK`. Setting the @ref status member has no
			 * effect after at least one byte of the body has been inserted into the
			 * response_ostream instance. */
			status_code &status;

			/** @brief Headers to send
			 *
			 * @remark Applications may insert one or more HTTP headers into the @ref
			 * headers member to send those headers as part of the response message.
			 * Setting the @ref headers member has no effect after at least one byte
			 * of the body has been inserted into the response_ostream instance. */
			header_map &headers;

		public:
			virtual ~response_ostream() {}
			response_ostream(std::streambuf *sb, status_code &scode, header_map &hdrs);
			response_ostream(response_ostream const &) = delete;
			response_ostream &operator=(response_ostream const &) = delete;
#ifndef CLANE_HAVE_NO_DEFAULT_MOVE
			response_ostream(response_ostream &&) = default;
			response_ostream &operator=(response_ostream &&) = default;
#endif
		};

		inline response_ostream::response_ostream(std::streambuf *sb, status_code &scode, header_map &hdrs):
		 	std::ostream{sb}, status(scode), headers(hdrs) {}

		struct server_options {

			server_options():
				max_header_size{8*1024},
				header_timeout{std::chrono::minutes{2}},
				read_timeout{std::chrono::minutes{2}},
				write_timeout{std::chrono::minutes{2}} {}

			size_t max_header_size;
			std::chrono::steady_clock::duration header_timeout;
			std::chrono::steady_clock::duration read_timeout;
			std::chrono::steady_clock::duration write_timeout;
		};

		class server_streambuf: public std::streambuf {
		private:
			net::socket conn;
			net::event &term_ev;
			server_options const *sopts;
		public:
			header_map *in_trls;
		private:
			bool hdrs_written;
			bool in_body;
			bool chunked;
			size_t content_len;

			// input buffer offsets:
			size_t ibeg;
			size_t iend;

			v1x_request_incparser pars;


			// response attributes:
			int out_major_ver;
			int out_minor_ver;
		public:
			status_code out_scode;
			header_map out_hdrs;
		private:

			char ibuf[4096]; // input buffer
			char obuf[4096]; // output buffer

		public:
			virtual ~server_streambuf();
			server_streambuf(net::socket &&conn, net::event &term_ev, server_options const *sopts);
			server_streambuf(server_streambuf const &) = default;
			server_streambuf &operator=(server_streambuf const &) = default;
#ifndef CLANE_HAVE_NO_DEFAULT_MOVE
			server_streambuf(server_streambuf &&) = default;
			server_streambuf &operator=(server_streambuf &&) = default;
#endif

			// Returns true if and only if the headers have been received, false if
			// and only if the connection should close. By returning whether the
			// headers have been received (and not invoking the root handler for the
			// request), the basic_server<> template class may call this function
			// without the function definition residing in the header file, a la
			// pimpl-lite. The cost, however, is a little more code complexity. The
			// server_streambuf class has two contexts: (1) when it's receiving the
			// headers, in which case the stream buffer is ignored, and (2) when it's
			// invoked via virtual std::streambuf methods to obtain more body data, in
			// which case the stream buffer is used.
			bool recv_header();

			// request accessors:
			// These become available when the recv_header() method returns true.
			std::string &method() { return pars.method(); }
			uri::uri &uri() { return pars.uri(); }
			int major_version() { return pars.major_version(); }
			int minor_version() { return pars.minor_version(); }
			header_map &headers() { return pars.headers(); }
			header_map &trailers() { return pars.trailers(); }

			bool headers_written() const { return hdrs_written; }

		protected:
			virtual int_type underflow();
			virtual int sync();
			virtual int_type overflow(int_type ch);
		private:
			bool recv_some();
			size_t parse_some();
			int flush(bool end = false);
		};

		/** @brief Perform post-processing for a server root handler
		 *
		 * @remark Applications should not use the post_handler() function directly.
		 *
		 * @sa basic_server */
		void post_handler(server_streambuf &sb, response_ostream &rs, request &req);

		/** @brief HTTP server
		 *
		 * @tparam Handler An @ref http_request_handling_page "HTTP request handler"
		 *
		 * @remark A basic_server instance continually serves connections accepted
		 * from a set of listeners. The server hides the details of handling the
		 * network I/O and message parsing and calls a **root handler** once for
		 * each valid incoming request. The root handler generates the response for
		 * the request, possibly by calling another handler to do the work.
		 *
		 * @remark To set up a server, applications should:
		 *
		 * @remark <ol> <li>Construct a basic_server instance,
		 * <li>Set the server's root handler,
		 * <li>Add one or more listeners, and
		 * <li>Call the server's serve() method.
		 * </ol>
		 *
		 * @remark The server instance's serve() method blocks until the server
		 * terminates, either due to an unrecoverable error or via its terminate()
		 * method. As such, applications must call serve() within a dedicated
		 * thread. Internally, the server threading model is naive: it spawns a
		 * unique thread for each listener, for each connection, and for each
		 * incoming request. Future versions of @projectname may use fewer threads.
		 *
		 * @remark A basic_server is a template based on the handler type.
		 * @projectname also provides the non-templated http::server type, which
		 * uses `std::function` for its request handler type.
		 *
		 * @sa http::server
		 * @sa make_server()
		 *
		 */
		template <typename Handler> class basic_server {
			static size_t const default_max_header_size = 8 * 1024;
			std::deque<clane::net::socket> listeners;
			clane::net::event term_ev;
			std::deque<std::thread> thrds;
			clane::sync::wait_group *conn_wg; // XXX: what to do with this?
		public:

			/** @brief @ref http_request_handling_page "HTTP request handler" that
			 * receives all incoming requests
			 *
			 * @remark The server calls its **root handler** for each incoming HTTP
			 * request. The root handler may respond to the request directly or else
			 * dispatch the request to a sub-handler to do the work. Either way, when
			 * the root handler returns, the application has handled the request.
			 *
			 * @remark As a special case, if the root handler returns and (1) the
			 * response error status code @link denotes_error() denotes an error
			 * @endlink and (2) the response body is empty and not flushed then the
			 * server will add human-readable body content describing the error. This
			 * causes applications by default to send human-meaningful error
			 * responses---even errors originating from built-in handlers, such as
			 * basic_router---while allowing applications to customize all error
			 * responses. */
			Handler root_handler;

			/** @brief Options to control server behavior */
			server_options options;

		public:
			~basic_server() = default;
			basic_server() {}
			basic_server(Handler &&h);
			basic_server(basic_server const &) = delete;
			basic_server &operator=(basic_server const &) = delete;
#ifndef CLANE_HAVE_NO_DEFAULT_MOVE
			basic_server(basic_server &&) = default;
			basic_server &operator=(basic_server &&) = default;
#else
			basic_server(basic_server &&that) noexcept;
			basic_server &operator=(basic_server &&that) noexcept;
#endif

			void add_listener(char const *addr);
			void add_listener(std::string const &addr);
			void add_listener(net::socket &&lis);

			/** @brief Run the server
			 *
			 * @remark The serve() method runs the server in the current thread. The
			 * serve() method returns only after the server has terminated, either
			 * due to an unrecoverable error or via the terminate() method. If an
			 * unrecoverable error occurs then the serve() method throws an exception.
			 *
			 * @sa terminate() */
			void serve();

			/** @brief Stops the server
			 *
			 * @remark The terminate() method puts the server into a terminal state.
			 * If the server is currently running then it will begin shutting down. If
			 * instead the application starts the server in the future then the server
			 * will immediately terminate after it starts. The terminal state is
			 * permanent: a server cannot run twice.
			 *
			 * @remark When in a terminal state, the server doesn't accept new network
			 * connections and doesn't receive new HTTP requests on existing
			 * connections. If a connection has any outstanding requests—i.e., any
			 * requests currently being handled—then the server waits for all of those
			 * requests' root handler invocations to return before closing the
			 * connection. Once all connections have closed, the server frees
			 * remaining resources and terminates.
			 *
			 * @remark The terminate() method returns immediately, possibly before the
			 * server terminates. To determine when the server has terminated, an
			 * application should check for the return of its call to the serve()
			 * method.
			 *
			 * @sa serve() */
			void terminate();

		private:
			void listen_main(net::socket &&lis);
#ifdef CLANE_HAVE_NO_STD_THREAD_MOVE_ARG
			void listen_main_move_wrapper(std::mutex &lis_mutex, std::condition_variable &lis_cond, net::socket **lis);
#endif
			void connection_main(net::socket conn);
#ifdef CLANE_HAVE_NO_STD_THREAD_MOVE_ARG
			void connection_main_move_wrapper(std::mutex &conn_mutex, std::condition_variable &conn_cond, net::socket **conn);
#endif
		};

		/** @brief Specializes basic_server for a `std::function` request handler
		 *
		 * @remark The @ref server type may be easier to use than the basic_server
		 * template type but may incur additional runtime cost when otherwise not
		 * using a `std::function` handler type.
		 *
		 * @sa basic_server
		 * @sa make_server() */
		typedef basic_server<std::function<void(response_ostream &, request &)>> server;

		/** @brief Constructs and returns a basic_server instance
		 *
		 * @relatesalso basic_server
		 *
		 * @remark The make_server() function is a convenience function that allows
		 * an application to create a basic_server instance using template type
		 * deduction for the request handler type.
		 *
		 * @sa basic_server
		 * @sa server */
		template <typename Handler> basic_server<Handler> make_server(Handler &&h) {
			return basic_server<Handler>(std::forward<Handler>(h));
		}

		template <typename Handler> basic_server<Handler>::basic_server(Handler &&h):
			root_handler{std::forward<Handler>(h)} {}

#ifdef CLANE_HAVE_NO_DEFAULT_MOVE

		template <typename Handler> basic_server<Handler>::basic_server(basic_server &&that) noexcept:
			root_handler{std::move(that.root_handler)},
			options{std::move(that.options)} {}

		template <typename Handler> basic_server<Handler> &basic_server<Handler>::operator=(basic_server &&that) noexcept {	
			root_handler = std::move(that.root_handler);
			options = std::move(that.options);
			return *this;
		}

#endif

		template <typename Handler> void basic_server<Handler>::add_listener(char const *addr) {
			listeners.push_back(listen(&net::tcp, addr));
			listeners.back().set_nonblocking();
		}

		template <typename Handler> void basic_server<Handler>::add_listener(std::string const &addr) {
			listeners.push_back(listen(&net::tcp, addr));
			listeners.back().set_nonblocking();
		}

		template <typename Handler> void basic_server<Handler>::add_listener(net::socket &&lis) {
			lis.set_nonblocking();
			listeners.push_back(std::move(lis));
		}

		template <typename Handler> void basic_server<Handler>::serve() {

			sync::wait_group wg; // for waiting on connections to stop
			conn_wg = &wg;

			// unique thread for each listener:

			while (!listeners.empty()) {
#ifndef CLANE_HAVE_NO_STD_THREAD_MOVE_ARG
				thrds.push_back(std::thread(&basic_server::listen_main, this, std::move(listeners.front())));
#else
				// If std::thread doesn't support rvalue reference arguments then move
				// the socket from within the thread.
				net::socket lis = std::move(listeners.front());
				net::socket *plis = &lis;
				std::mutex lis_mutex;
				std::condition_variable lis_cond;
				std::unique_lock<std::mutex> lis_lock(lis_mutex);
				thrds.push_back(std::thread(&basic_server::listen_main_move_wrapper, this, std::ref(lis_mutex), std::ref(lis_cond), &plis));
				while (plis)
					lis_cond.wait(lis_lock);
#endif
				listeners.pop_front();
			}

			// wait for the termination signal:
			net::poller poller;
			poller.add(term_ev, poller.in);
			poller.poll();

			// wait for all listeners to stop:
			for (auto i = thrds.begin(); i != thrds.end(); ++i)
				i->join();
			thrds.clear();

			// The connection wait group will cause this thread to block until
			// all connection threads have stopped.
		}

		template <typename Handler> void basic_server<Handler>::terminate() {
			term_ev.signal();
		}

#ifndef CLANE_HAVE_STD_THREAD_MOVE_ARG
		template <typename Handler> void basic_server<Handler>::listen_main_move_wrapper(std::mutex &lis_mutex,
		std::condition_variable &lis_cond, net::socket **lis) {
			std::unique_lock<std::mutex> lis_lock(lis_mutex);
			net::socket mlis = std::move(**lis);
			*lis = nullptr;
			lis_cond.notify_one();
			lis_lock.unlock();
			listen_main(std::move(mlis));
		}
#endif

		template <typename Handler> void basic_server<Handler>::listen_main(net::socket &&lis) {

			net::poller poller;
			size_t const iterm = poller.add(term_ev, poller.in);
			poller.add(lis, poller.in);

			// accept incoming connections, and launch a unique thread for each new
			// connection:
			auto poll_res = poller.poll();
			while (poll_res.index != iterm) {
				std::error_code e;
				net::socket conn = lis.accept(e);
				if (e)
					continue; // ignore error
#ifndef CLANE_HAVE_NO_STD_THREAD_MOVE_ARG
				std::thread conn_thrd(&basic_server::connection_main, this, std::move(conn));
#else
				// If std::thread doesn't support rvalue reference arguments then move
				// the socket from within the thread.
				net::socket *pconn = &conn;
				std::mutex conn_mutex;
				std::condition_variable conn_cond;
				std::unique_lock<std::mutex> conn_lock(conn_mutex);
				std::thread conn_thrd(&basic_server::connection_main_move_wrapper, this, std::ref(conn_mutex), std::ref(conn_cond), &pconn);
				while (pconn)
					conn_cond.wait(conn_lock);
#endif
				conn_thrd.detach();
				poll_res = poller.poll();
			}
		}

#ifdef CLANE_HAVE_NO_STD_THREAD_MOVE_ARG
		template <typename Handler> void basic_server<Handler>::connection_main_move_wrapper(std::mutex &conn_mutex,
		std::condition_variable &conn_cond, net::socket **conn) {
			std::unique_lock<std::mutex> conn_lock(conn_mutex);
			net::socket mconn = std::move(**conn);
			*conn = nullptr;
			conn_cond.notify_one();
			conn_lock.unlock();
			connection_main(std::move(mconn));
		}
#endif

		template <typename Handler> void basic_server<Handler>::connection_main(net::socket conn) {

			conn.set_nonblocking();
			server_streambuf sb{std::move(conn), term_ev, &options};

			while (true) {

				// receive the headers for the next request:
				bool done = !sb.recv_header();
				if (done)
					return;

				// launch the root handler:
				response_ostream rs{&sb, sb.out_scode, sb.out_hdrs};
				request req{&sb};
				sb.in_trls = &req.trailers;
				req.method = std::move(sb.method());
				req.uri = std::move(sb.uri());
				req.major_version = std::move(sb.major_version());
				req.headers = std::move(sb.headers());
				root_handler(rs, req);

				// post-processing:
				post_handler(sb, rs, req);
			}
		}

	}
}

#endif // #ifndef CLANE_HTTP_SERVER_HPP
