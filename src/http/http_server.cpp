// vim: set noet:

#include "http_consume.hpp"
#include "http_server.hpp"
#include "../net/net_inet.hpp"
#include "../net/net_poll.hpp"
#include <algorithm>
#include <sstream>

namespace clane {
	namespace http {

		// TODO: move to generic sublibrary
		// TODO: unit test
		class reference_counter {
		public:
			class reference {
				reference_counter *rc;
			public:
				~reference() { if (rc) { rc->decrement(); }}
				reference(reference_counter *rc) noexcept: rc{rc} {}
				reference(reference const &) = delete;
				reference(reference &&that) noexcept: rc{} { swap(that); }
				reference &operator=(reference const &) = delete;
				reference &operator=(reference &&that) noexcept { swap(that); return *this; }
				void swap(reference &that) noexcept { std::swap(rc, that.rc); }
			};
		private:
			int cnt;
			std::mutex mutex;
			std::condition_variable cond;
		public:
			~reference_counter();
			reference_counter(): cnt{} {}
			reference_counter(reference_counter const &) = delete;
			reference_counter(reference_counter &&) = delete;
			reference_counter &operator=(reference_counter const &) = delete;
			reference_counter &operator=(reference_counter &&) = delete;
			reference new_reference();
		private:
			void decrement();
		};

		reference_counter::~reference_counter() {
			// wait for the count to become zero:
			std::unique_lock<std::mutex> lock(mutex);
			cond.wait(lock, [&]() -> bool { return 0 == cnt; });
		}

		reference_counter::reference reference_counter::new_reference() {
			{
				std::lock_guard<std::mutex> lock(mutex);
				++cnt;
			}
			return reference(this);
		};

		void reference_counter::decrement() {
			std::lock_guard<std::mutex> lock(mutex);
			--cnt;
			if (0 == cnt)
				cond.notify_one();
		}

		class server_streambuf: public std::streambuf {
		public:
			virtual ~server_streambuf() = default;
			server_streambuf() = default;
			server_streambuf(server_streambuf const &) = default;
			server_streambuf(server_streambuf &&) = default;
			server_streambuf &operator=(server_streambuf const &) = default;
			server_streambuf &operator=(server_streambuf &&) = default;
			void append_request_body(std::shared_ptr<char> p, size_t offset, size_t size);
			// TODO: streambuf overloads
		};

		void server_streambuf::append_request_body(std::shared_ptr<char> p, size_t offset, size_t size) {
			// TODO: implement
		}

		static bool chunked_xfer_encoding(header_map const &hdrs) {
			// FIXME: Ignore transfer-encoding order. Assume that if any one of the
			// transfer-encoding headers is "chunked" then the message is chunked.
			static std::string const key("transfer-encoding");
			auto r = hdrs.equal_range(key);
			return r.second != std::find_if(r.first, r.second, [&](std::pair<std::string, std::string> const &v) {
					return v.second == "chunked";
					});
		}

		static bool content_length(header_map const &hdrs, size_t &content_len) {
			// FIXME: Ignore multiple and invalid content-length headers. Assume the
			// first content-length header 
			static std::string const key("content-length");
			auto p = hdrs.find(key);
			if (p == hdrs.end())
				return false;
			std::istringstream ss(p->second);
			size_t len;
			ss >> len;
			if (!ss || !ss.eof())
				return false;
			content_len = len;
			return true;
		}

		server::scoped_conn_ref::~scoped_conn_ref() {
			if (ser) {
				std::lock_guard<std::mutex> l(ser->conn_cnt_mutex);
				--ser->conn_cnt;
				if (!ser->conn_cnt)
					ser->conn_cnt_cond.notify_all();
			}
		}

		server::scoped_conn_ref::scoped_conn_ref(server *ser): ser(ser) {
			std::lock_guard<std::mutex> l(ser->conn_cnt_mutex);
			++ser->conn_cnt;
		}

		void server::add_listener(char const *addr) {
			listeners.push_back(listen(&net::tcp, addr));
		}

		void server::add_listener(std::string const &addr) {
			listeners.push_back(listen(&net::tcp, addr));
		}

		void server::run() {

			// unique thread for each listener:
			while (!listeners.empty()) {
				thrds.push_back(std::thread(&server::listen_main, this, std::move(listeners.front())));
				listeners.pop_front();
			}

			// wait for the termination signal:
			net::poller poller;
			poller.add(term_event, poller.in);
			poller.poll();

			// wait for all listeners to stop:
			for (std::thread &p: thrds)
				p.join();

			// wait for all connections to stop:
			{
				std::unique_lock<std::mutex> l(conn_cnt_mutex);
				conn_cnt_cond.wait(l, [&]() -> bool { return !conn_cnt; });
			}

			// cleanup:
			thrds.clear();
			term_event.reset();
		}

		void server::terminate() {
			term_event.signal();
		}

		void server::listen_main(net::socket lis) {
			net::poller poller;
			size_t const iterm = poller.add(term_event, poller.in);
			poller.add(lis, poller.in);

			// accept incoming connections, and launch a unique thread for each new
			// connection:
			auto poll_res = poller.poll();
			while (poll_res.index != iterm) {
				auto accept_res = lis.accept();
				if (net::status::ok != accept_res.stat)
					continue;
				scoped_conn_ref conn_ref(this);
				std::thread conn_thrd(&server::connection_main, this, std::move(accept_res.sock), std::move(conn_ref));
				conn_thrd.detach();
				poll_res = poller.poll();
			}
		}

		void server::connection_main(net::socket conn, scoped_conn_ref ref) {

			reference_counter out_cnt; // last thing to destruct

			// input buffer:
			static size_t const incap = 4096;
			std::shared_ptr<char> inbuf;
			size_t inoff = incap;

			// request-handler context:
			class context {
				reference_counter::reference ref;
			public:
				server_streambuf sb;
				request req;
				oresponsestream rs;
			public:
				~context() = default;
				context(reference_counter &rc): ref{rc.new_reference()}, rs{&sb} {}
				context(context const &) = delete;
				context(context &&) = delete;
				context &operator=(context const &) = delete;
				context &operator=(context &&) = delete;
			};
			auto cur_ctx = std::make_shared<context>(out_cnt);

			// request-handler invoker:
			auto handler_thread_main = [&](std::shared_ptr<context> ctx) {
				root_handler(ctx->rs, ctx->req);
			};

			// parsing:
			request_1x_consumer req_cons(cur_ctx->req);
			req_cons.set_length_limit(max_header_size);
			enum class phase {
				head,
				body_fixed,      // Content-Length
				body_chunk_line, // Transfer-Encoding: chunked
				body_chunk,
				trailer,
				body_end,        // no length limit, body ends with connection
			} cur_phase = phase::head;
			size_t content_len;
			chunk_line_consumer chunk_cons;

			// I/O multiplexing:
			std::chrono::steady_clock::time_point to;
			if (std::chrono::steady_clock::duration::zero() != read_timeout)
				to = std::chrono::steady_clock::now() + read_timeout;
			net::poller poller;
			size_t const iterm = poller.add(term_event, poller.in);
			poller.add(conn, poller.in);

			// consume incoming data from the connection:
			bool done = false;
			while (!done) {

				// wait for event: data, termination, or timeout
				auto poll_res = std::chrono::steady_clock::duration::zero() == to.time_since_epoch() ? poller.poll() : poller.poll(to);
				if (!poll_res.index) {
					// FIXME: timeout
					break;
				}
				if (poll_res.index == iterm) {
					// FIXME: termination
					break;
				}

				// reallocate input buffer if full:
				if (inoff == incap) {
					inbuf = std::unique_ptr<char, std::default_delete<char>>(new char[incap]);
					inoff = 0;
				}

				// receive:
				auto xfer_res = conn.recv(reinterpret_cast<char *>(inbuf.get()) + inoff, incap - inoff);
				if (net::status::would_block == xfer_res.stat)
					continue; // go back to waiting
				if (net::status::ok != xfer_res.stat)
					// FIXME: connection error
					break;
				if (!xfer_res.size) {
					// FIXME: connection FIN
					break;
				}

				// process the received data:
				do {
					switch (cur_phase) {
						case phase::head: {
							size_t len = req_cons.length();
							if (!req_cons.consume(inbuf.get() + inoff, xfer_res.size)) {
								inoff += xfer_res.size;
								xfer_res.size = 0;
								break;
							}
							size_t delta = req_cons.length() - len;
							inoff += delta;
							xfer_res.size -= delta;
							if (!req_cons) {
								// FIXME: invoke error request handler
								done = true;
								break;
							}
							if (chunked_xfer_encoding(cur_ctx->req.headers)) {
								cur_phase = phase::body_chunk_line;
								chunk_cons.reset();
							} else if (content_length(cur_ctx->req.headers, content_len)) {
								cur_phase = phase::body_fixed;
							} else {
								cur_phase = phase::body_end;
							}
							// start request handler:
							std::thread thrd(handler_thread_main, cur_ctx);
							thrd.detach();
						}

						case phase::body_fixed: {
							size_t len = std::min(content_len, xfer_res.size);
							cur_ctx->sb.append_request_body(inbuf, inoff, xfer_res.size);
							inoff += len;
							xfer_res.size -= len;
							content_len -= len;
							if (!content_len) {
								cur_phase = phase::head;
								cur_ctx = std::make_shared<context>(out_cnt);
								req_cons.reset(cur_ctx->req);
							}
							break;
						}

						case phase::body_chunk_line: {
							size_t len = chunk_cons.length();
							if (!chunk_cons.consume(inbuf.get() + inoff, xfer_res.size)) {
								inoff += xfer_res.size;
								xfer_res.size = 0;
								break;
							}
							size_t delta = chunk_cons.length() - len;
							inoff += delta;
							xfer_res.size -= delta;
							if (!chunk_cons) {
								// FIXME: chunk line error
								done = true;
								break;
							}
							if (!chunk_cons.chunk_size()) {
								// TODO: add support for trailers
								cur_phase = phase::head;
								cur_ctx = std::make_shared<context>(out_cnt);
								req_cons.reset(cur_ctx->req);
							}
							cur_phase = phase::body_chunk;
							content_len = chunk_cons.chunk_size();
							break;
						}

						case phase::body_chunk: {
							size_t len = std::min(content_len, xfer_res.size);
							cur_ctx->sb.append_request_body(inbuf, inoff, xfer_res.size);
							inoff += len;
							xfer_res.size -= len;
							content_len -= len;
							if (!content_len) {
								cur_phase = phase::body_chunk_line;
								chunk_cons.reset();
							}
							break;
						}

						case phase::trailer:
							throw std::runtime_error("unimplemented");

						case phase::body_end:
							cur_ctx->sb.append_request_body(inbuf, inoff, xfer_res.size);
							inoff += xfer_res.size;
							xfer_res.size = 0;
							break;

					}
				} while (xfer_res.size);
			}
		}
	}
}

