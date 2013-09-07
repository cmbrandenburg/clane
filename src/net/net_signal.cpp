// vim: set noet:

/** \file */

#include "net_error.h"
#include "net_signal.h"
#include <sstream>
#include <sys/timerfd.h>

namespace clane {
	namespace net {

		mux_conn::~mux_conn() {
			delete[] ibuf;
		}

		void mux_conn::alloc_ibuffer() {
			icap = 4096;
			ibuf = new char[icap];
			ioffset = 0;
		}

		void mux_conn::dealloc_ibuffer() {
			delete[] ibuf;
			ibuf = nullptr;
		}

		int mux_conn::initial_readiness() const {
			return read_flag;
		}

		mux_conn::mux_conn(net::socket &&that_sock): mux_socket(std::move(that_sock)), ibuf(nullptr) {
		}

		mux_signal::ready_result mux_conn::read_ready() {
			static recv_options const recv_opts{true};
			if (!ibuf) {
				alloc_ibuffer();
			}
			recv_result istat = socket().recv(&ibuf[ioffset], icap - ioffset, &recv_opts);
			if (istat.reset) {
				return ready_result::signal_complete;
			}
			if (istat.shutdown) {
				finished();
				return ready_result::op_complete;
			}
			if (!istat.size) {
				return ready_result::op_complete; // receive operation would block
			}
			if (recv_some(ibuf, icap, ioffset, istat.size)) {
				ibuf = nullptr;
			} else {
				ioffset += istat.size;
				if (ioffset == icap) {
					dealloc_ibuffer();
				}
			}
			return ready_result::op_incomplete;
		}

		void mux_conn::send_all(void const *buf, size_t size) {
			size_t sent = 0;
			char const *pbuf = reinterpret_cast<char const *>(buf);
			while (sent < size) {
				send_result ostat = socket().send(&pbuf[sent], size - sent);
				if (ostat.reset)
					return; // connection is reset--drop outgoing data
				sent += ostat.size;
			}
		}

		void mux_conn::shutdown() {
			socket().shut_down(socket::shutdown_how::write);
		}

		int mux_listener::initial_readiness() const {
			return read_flag;
		}

		mux_listener::mux_listener(net::socket &&that_sock): mux_socket(std::move(that_sock)) {
			socket().set_nonblocking();
		}

		mux_signal::ready_result mux_listener::read_ready() {
			auto astat = accept();
			if (astat.aborted)
				return ready_result::op_incomplete;
			if (!astat.conn)
				return ready_result::op_complete; // accept blocked
			mux_owner->add_signal(astat.conn);
			return ready_result::op_incomplete;
		}

		mux_signal::ready_result mux_listener::write_ready() {
			throw std::runtime_error("write event on listening socket");
		}

		mux_server_conn::mux_server_conn(net::socket &&that_sock): mux_conn(std::move(that_sock)) {
		}

		mux_signal::ready_result mux_server_conn::write_ready() {
			// Server connections are always in a connected state, and send operations
			// always block, so there's nothing to do for write-readiness.
			return ready_result::op_complete;
		}

		file_descriptor const &mux_socket::fd() const {
			return sock_;
		}

		mux_socket::mux_socket(net::socket &&that_sock): sock_(std::move(that_sock)) {
		}

		file_descriptor const &mux_timer::fd() const {
			return fd_;
		}

		mux_timer::mux_timer(std::function<void()> signalee): signalee(signalee) {
			fd_ = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK);
			if (-1 == fd_) {
				std::ostringstream ss;
				ss << "error creating timer object: " << safe_strerror(errno);
				throw std::runtime_error(ss.str());
			}
		}

		mux_signal::ready_result mux_timer::read_ready() {
			itimerspec t;
			int status;
			do {
				status = timerfd_gettime(fd_, &t);
			} while (-1 == status && EINTR == errno);
			if (-1 == status && EAGAIN == errno)
				return ready_result::op_complete;
			if (-1 == status) {
				std::ostringstream ss;
				ss << "error reading timer object: " << safe_strerror(errno);
				throw std::runtime_error(ss.str());
			}
			signalee();
			return ready_result::op_complete;
		}

		void mux_timer::set(std::chrono::steady_clock::duration const &to) {
			itimerspec t{};
			t.it_value.tv_sec = to / std::chrono::seconds(1);
			t.it_value.tv_nsec %= (to % std::chrono::seconds(1)) * 1000000000 / std::chrono::nanoseconds(1);
			int status = timerfd_settime(fd_, 0, &t, nullptr);
			if (-1 == status) {
				std::ostringstream ss;
				ss << "error setting timer object: " << safe_strerror(errno);
				throw std::runtime_error(ss.str());
			}
		}

		mux_signal::ready_result mux_timer::write_ready() {
			return ready_result::op_complete;
		}

	}
}

