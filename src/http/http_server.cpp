// vim: set noet:

#include "http_server.hpp"
#include "../net/net_inet.hpp"
#include "../net/net_poll.hpp"

namespace clane {
	namespace http {

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

			// wait for all connections to stop:
		}

		void server::connection_main(net::socket conn, scoped_conn_ref ref) {
			static size_t const incap = 4096;
			std::shared_ptr<char> inbuf;
			size_t inoff = incap;

			net::poller poller;
			size_t const iterm = poller.add(term_event, poller.in);
			poller.add(conn, poller.in);

			// consume incoming data from the connection:
			while (true) {

				// wait for event:
				// FIXME: current read timeout
				auto poll_res = poller.poll();
				if (poll_res.index == iterm)
					break; // termination signaled

				// reallocate input buffer if full:
				if (inoff == incap) {
					inbuf = std::unique_ptr<char, std::default_delete<char>>(new char[incap]);
					inoff = 0;
				}

				// receive:
				auto xfer_res = conn.recv(reinterpret_cast<char *>(inbuf.get()) + inoff, incap - inoff);
				if (net::status::would_block == xfer_res.stat)
					continue;
				if (net::status::ok != xfer_res.stat)
					return;
				if (!xfer_res.size) {
					// FIXME: handle FIN
					return;
				}
				inoff += xfer_res.size;

			}
		}
	}
}

