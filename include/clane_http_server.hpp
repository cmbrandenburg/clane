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
			response_ostream(std::streambuf *sb, status_code &stat_code, header_map &hdrs);
			response_ostream(response_ostream const &) = delete;
			response_ostream &operator=(response_ostream const &) = delete;
#ifndef CLANE_HAVE_NO_DEFAULT_MOVE
			response_ostream(response_ostream &&) = default;
			response_ostream &operator=(response_ostream &&) = default;
#endif
		};

		inline response_ostream::response_ostream(std::streambuf *sb, status_code &stat_code, header_map &hdrs):
		 	std::ostream{sb}, status(stat_code), headers(hdrs) {}

		class server_streambuf: public std::streambuf {
			struct buffer {
				std::shared_ptr<char> p;
				size_t size;
			};
			net::socket &sock;
			int major_ver;
			int minor_ver;
		public:
			status_code out_stat_code;
			header_map out_hdrs;
		private:
			std::mutex in_mutex;
			bool in_end;
			std::deque<buffer> in_queue;
			std::condition_variable in_cond;
			std::mutex act_mutex;
			std::condition_variable act_cond;
			bool enabled;
			bool active;
			bool hdrs_written;
			bool chunked;
			size_t content_len;
			char out_buf[4096];
		public:
			virtual ~server_streambuf();
			server_streambuf(net::socket &sock);
			server_streambuf(server_streambuf const &) = default;
			server_streambuf &operator=(server_streambuf const &) = default;
#ifndef CLANE_HAVE_NO_DEFAULT_MOVE
			server_streambuf(server_streambuf &&) = default;
			server_streambuf &operator=(server_streambuf &&) = default;
#endif
			void enable() { enabled = true; }
			void set_version(int major, int minor) { major_ver = major; minor_ver = minor; }
			void more_request_body(std::shared_ptr<char> const &p, size_t offset, size_t size);
			void end_request_body();
			void inactivate();
			void activate();
			bool headers_written() const { return hdrs_written; }
		protected:
			virtual int sync();
			virtual int_type underflow();
			virtual int_type overflow(int_type ch);
		private:
			int flush(bool end = false);
		};

		class server_context {
			sync::wait_group::reference wg_ref;
		public:
			server_streambuf sb;
			request req;
			response_ostream rs;
		private:
			std::mutex next_mutex;
			std::shared_ptr<server_context> next_ctx;
		public:
			~server_context();
			server_context(sync::wait_group::reference &&wg_ref, net::socket &sock): wg_ref{std::move(wg_ref)}, sb{sock},
				req{&sb}, rs{&sb, sb.out_stat_code, sb.out_hdrs} {}
			server_context(server_context const &) = delete;
			server_context(server_context &&) = delete;
			server_context &operator=(server_context const &) = delete;
			server_context &operator=(server_context &&) = delete;
			void set_next_context(std::shared_ptr<server_context> const &nc);
		private:
			void activate();
		};

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
			clane::net::event term_event;
			std::deque<std::thread> thrds;
			clane::sync::wait_group *conn_wg;
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

			size_t max_header_size;
			std::chrono::steady_clock::duration read_timeout;
			std::chrono::steady_clock::duration write_timeout;

		public:
			~basic_server() = default;
			basic_server();
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
#ifndef CLANE_HAVE_STD_THREAD_MOVE_ARG
			void listen_main_move_wrapper(std::mutex &lis_mutex, std::condition_variable &lis_cond, net::socket **lis);
#endif
			void connection_main(net::socket &&conn);
#ifndef CLANE_HAVE_STD_THREAD_MOVE_ARG
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

		template <typename Handler> basic_server<Handler>::basic_server():
			max_header_size{default_max_header_size},
			read_timeout{0},
			write_timeout{0} {}

		template <typename Handler> basic_server<Handler>::basic_server(Handler &&h):
			root_handler{std::forward<Handler>(h)},
			max_header_size{default_max_header_size},
			read_timeout{0},
			write_timeout{0} {}

#ifdef CLANE_HAVE_NO_DEFAULT_MOVE

		template <typename Handler> basic_server<Handler>::basic_server(basic_server &&that) noexcept:
			root_handler{std::move(that.root_handler)},
			max_header_size{std::move(that.max_header_size)},
			read_timeout{std::move(that.read_timeout)},
			write_timeout{std::move(that.write_timeout)} {}

		template <typename Handler> basic_server<Handler> &basic_server<Handler>::operator=(basic_server &&that) noexcept {	
			root_handler = std::move(that.root_handler);
			max_header_size = std::move(that.max_header_size);
			read_timeout = std::move(that.read_timeout);
			write_timeout = std::move(that.write_timeout);
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
#ifdef CLANE_HAVE_STD_THREAD_MOVE_ARG
				thrds.push_back(std::thread(&basic_server::listen_main, this, std::move(listeners.front())));
#else
				// If std::thread doesn't support rvalue reference arguments then move
				// the socket inside the thread.
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
			poller.add(term_event, poller.in);
			poller.poll();

			// wait for all listeners to stop:
			for (auto i = thrds.begin(); i != thrds.end(); ++i)
				i->join();
			thrds.clear();

			// The connection wait group will cause this thread to block until
			// all connection threads have stopped.
		}

		template <typename Handler> void basic_server<Handler>::terminate() {
			term_event.signal();
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
			size_t const iterm = poller.add(term_event, poller.in);
			poller.add(lis, poller.in);

			// accept incoming connections, and launch a unique thread for each new
			// connection:
			auto poll_res = poller.poll();
			while (poll_res.index != iterm) {
				std::error_code e;
				net::socket conn = lis.accept(e);
				if (e)
					continue; // ignore error
#ifdef CLANE_HAVE_STD_THREAD_MOVE_ARG
				std::thread conn_thrd(&basic_server::connection_main, this, std::move(conn));
#else
				// If std::thread doesn't support rvalue reference arguments then move
				// the socket inside the thread.
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

#ifndef CLANE_HAVE_STD_THREAD_MOVE_ARG
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

		template <typename Handler> void handler_main(Handler &h, std::shared_ptr<server_context> ctx) {

			// root handler:
			h(ctx->rs, ctx->req);

			// Special case: If the root handler responds with (1) an error status
			// code (4xx or 5xx) and (2) an empty and non-flushed body then fill out
			// the body with the status code reason phrase. This causes error
			// responses to be human-meaningful by default but still allows
			// applications to customize the error response page--even for error
			// responses generated by built-in handlers such as the basic_router.

			if (!ctx->sb.headers_written() && denotes_error(ctx->rs.status)) {
				auto r = ctx->rs.headers.equal_range("content-type");
				ctx->rs.headers.erase(r.first, r.second);
#ifdef CLANE_HAVE_STD_MULTIMAP_EMPLACE
				ctx->rs.headers.emplace("content-type", "text/plain");
#else
				ctx->rs.headers.insert(http::header("content-type", "text/plain"));
#endif
				ctx->rs << what(ctx->rs.status) << '\n';
			}
		}

		template <typename Handler> void basic_server<Handler>::connection_main(net::socket &&conn) {

			auto my_ref = conn_wg->new_reference();
			sync::wait_group req_wg; // for waiting on request-handler threads to complete

			conn.set_nonblocking();

			// input buffer:
			static size_t const incap = 4096;
			std::shared_ptr<char> inbuf;
			size_t inoff = incap;
			size_t insiz;

			// request-handler context:
			auto cur_ctx = std::make_shared<server_context>(req_wg.new_reference(), conn);

			// parsing:
			v1x_request_incparser pars;
			pars.reset();
			pars.set_length_limit(max_header_size);
			bool got_hdrs = false;

			// I/O multiplexing:
			std::chrono::steady_clock::time_point to;
			if (std::chrono::steady_clock::duration::zero() != read_timeout)
				to = std::chrono::steady_clock::now() + read_timeout;
			net::poller poller;
			size_t const iterm = poller.add(term_event, poller.in);
			poller.add(conn, poller.in);

			// consume incoming data from the connection:
			while (true) {

				// wait for event: data, termination, or timeout
				auto poll_res = std::chrono::steady_clock::duration::zero() == to.time_since_epoch() ? poller.poll() : poller.poll(to);
				if (!poll_res.index) {
					// FIXME: timeout
					goto done;
				}
				if (poll_res.index == iterm) {
					// FIXME: termination
					goto done;
				}

				// reallocate input buffer if full:
				if (inoff == incap) {
					inbuf = std::unique_ptr<char, std::default_delete<char[]>>(new char[incap]);
					inoff = insiz = 0;
				}

				// receive:
				{
					std::error_code e;
					size_t xstat = conn.recv(reinterpret_cast<char *>(inbuf.get()) + inoff, incap - inoff, e);
					if (e == std::errc::operation_would_block || e == std::errc::resource_unavailable_try_again)
						continue; // go back to waiting
					if (e) {
						// FIXME: connection error
						goto done;
					}
					if (!xstat) {
						// FIXME: connection FIN
						goto done;
					}
					insiz = xstat;
				}

				// process the received data:
				while (insiz) {

					// parse:
					size_t pstat = pars.parse_some(inbuf.get()+inoff, inbuf.get()+inoff+insiz);
					if (pars.error == pstat) {
						// FIXME: error
						goto done;
					}

					// FIXME: check HTTP version

					if (pars.got_headers()) {

						// got new request?
						if (!got_hdrs) {

							// set up request object:
							got_hdrs = true;
							cur_ctx->sb.enable();
							cur_ctx->req.method = std::move(pars.method());
							cur_ctx->req.uri = std::move(pars.uri());
							cur_ctx->req.major_version = pars.major_version();
							cur_ctx->req.minor_version = pars.minor_version();
							cur_ctx->sb.set_version(cur_ctx->req.major_version, cur_ctx->req.minor_version);
							cur_ctx->req.headers = std::move(pars.headers());

							// start request handler:
							std::thread(&handler_main<Handler>, std::ref(root_handler), cur_ctx).detach();
						}

						// feed body data to request object:
						cur_ctx->sb.more_request_body(inbuf, inoff+pars.offset(), pars.size());
					}

					inoff += pstat;
					insiz -= pstat;

					if (pars)
						continue;

					// request is complete:
					cur_ctx->req.trailers = std::move(pars.trailers());
					cur_ctx->sb.end_request_body();

					// prepare for next request:
					{
						auto next_ctx = std::make_shared<server_context>(req_wg.new_reference(), conn);
						cur_ctx->set_next_context(next_ctx); // set up pipeline dependency
						cur_ctx = std::move(next_ctx);
						pars.reset();
						got_hdrs = false;
					}
				}
			}
done: // connection is finished, regardless whether graceful or not
			{}
		}

	}
}

#endif // #ifndef CLANE_HTTP_SERVER_HPP
