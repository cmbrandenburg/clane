// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// vim: set noet:

/** @file */

#include "clane_http_parse.hpp"
#include "clane_http_server.hpp"
#include "../net/clane_net_inet.hpp"
#include "../net/clane_net_poller.hpp"
#include <cstring>

namespace clane {
	namespace http {

		class server_streambuf: public std::streambuf {

			friend class server_request_impl;

			server_connection *conn;
			server_options const *opts;
			header_map *in_trls;

			v1x_request_incparser pars;
			bool enabled;
			bool hdrs_written;
			bool chunked;
			size_t content_len;

			// response attributes:
			int out_major_ver;
			int out_minor_ver;
			status_code out_scode;
			header_map out_hdrs;

			char *ocur;
			char obuf[4096];

		public:
			virtual ~server_streambuf();
			server_streambuf(server_connection *conn, server_options const *opts);
			server_streambuf(server_streambuf const &) = default;
			server_streambuf &operator=(server_streambuf const &) = default;
#ifndef CLANE_HAVE_NO_DEFAULT_MOVE
			server_streambuf(server_streambuf &&) = default;
			server_streambuf &operator=(server_streambuf &&) = default;
#endif

		protected:
			virtual int_type underflow();
			virtual int sync();
			virtual int_type overflow(int_type ch);
		private:
			bool flush(bool end = false);
			bool headers_written() const { return hdrs_written; }
			void enable();
		};

		server_streambuf::~server_streambuf() {
			flush(true);
		}

		server_streambuf::server_streambuf(server_connection *conn, server_options const *opts):
			conn(conn),
			opts(opts),
			in_trls{},
			enabled{},
			hdrs_written{},
			ocur(obuf)
		{
			pars.set_length_limit(opts->max_header_size);
			pars.reset();
			setp(nullptr, nullptr); // force overflow on first write
		}

		server_streambuf::int_type server_streambuf::underflow() {
			while (gptr() >= egptr()) {

				if (!conn->recv_if_none_avail())
					return traits_type::eof(); // connection error

				// parse:
				size_t pstat = pars.parse_some(conn->icur(), conn->iend());
				if (pars.error == pstat) {
					// FIXME: Send error response to client?
					return traits_type::eof(); // invalid request body
				}
				if (!pars) {
					// request is completely received and parsed:
					*in_trls = std::move(pars.trailers());
					if (!pars.size())
						return traits_type::eof(); // no body
				}
				if (pars.size()) {
					char *p = conn->icur()+pars.offset();
					setg(conn->ibeg(), p, p+pars.size());
					conn->ibump(pstat);
					break;
				}
			}
			return traits_type::to_int_type(*gptr()); // success
		}

		int server_streambuf::sync() {
			return flush() ? 0 : -1;
		}

		server_streambuf::int_type server_streambuf::overflow(int_type ch) {
			if (!flush())
				return traits_type::eof(); // error
			setp(obuf, obuf+sizeof(obuf));
			if (traits_type::eof() != ch) {
				*pptr() = traits_type::to_char_type(ch);
				pbump(1);
			}
			return !traits_type::eof(); // success
		}

		bool server_streambuf::flush(bool end) {

			if (!enabled)
				return true;

			// FIXME: Each send operation has its own relative timeout. Because a
			// single flush operation may comprise multiple send operations, the
			// timeout value isn't reliable. The server option write_timeout should
			// apply cumulatively to all send operations.

			// TODO: Use scatter-gather I/O for combining send operations.

			// write headers if not already written:
			if (!hdrs_written) {

				// chunked?
				chunked = !query_headers_content_length(out_hdrs, content_len);
				if (chunked) {
#ifdef CLANE_HAVE_STD_MULTIMAP_EMPLACE
					out_hdrs.emplace("transfer-encoding", "chunked");
#else
					out_hdrs.insert(header("transfer-encoding", "chunked"));
#endif
				}

				// format status line and headers:
				std::ostringstream ss;
				ss << "HTTP/" << out_major_ver << '.' << out_minor_ver << ' ' << static_cast<int>(out_scode) << ' ' <<
					what(out_scode) << "\r\n";
				for (auto i = out_hdrs.begin(); i != out_hdrs.end(); ++i)
					ss << canonize_1x_header_name(i->first) << ": " << i->second << "\r\n";
				ss << "\r\n";
				std::string hdr_lines = ss.str();
				if (!conn->send_all(hdr_lines.data(), hdr_lines.size()))
					return false; // connection error

				hdrs_written = true;
			}

			// send chunk:
			size_t chunk_len = pptr() - pbase();
			if (chunk_len) {
				if (chunked) {
					std::ostringstream ss;
					ss << std::hex << chunk_len << "\r\n";
					std::string chunk_line = ss.str();
					if (!conn->send_all(chunk_line.data(), chunk_line.size()))
						return false; // connection error
				} else {
					// Sanity check: Ensure no request handler writes too many content
					// bytes. This may catch some application bugs.
					if (chunk_len > content_len)
						throw std::runtime_error("HTTP request handler wrote too much content");
					content_len -= chunk_len;
				}
				if (!conn->send_all(pbase(), chunk_len))
					return false; // connection error
				if (chunked) {
					if (!conn->send_all("\r\n", 2))
						return false; // connection error
				}
			}

			// final chunk:
			if (end && chunked) {
				static char const *const final_chunk = "0\r\n\r\n";
				if (!conn->send_all(final_chunk, std::strlen(final_chunk)))
					return false; // connection error
			} else if (end) {
				// Sanity check: Ensure no request handler writes too few content bytes.
				// This may catch some application bugs.
				if (content_len)
					throw std::runtime_error("HTTP request handler wrote too little content");
			}

			return true; // success
		}

		void server_streambuf::enable() {
			enabled = true;
		}

		class server_request_impl {
			server_streambuf sb;
		public:
			response_ostream rs;
			request req;
		public:
			~server_request_impl();
			server_request_impl(server_connection *conn, server_options const *opts);
			server_request_impl(server_request_impl const &) = delete;
			server_request_impl &operator=(server_request_impl const &) = delete;
			server_request_impl(server_request_impl &&) = delete;
			server_request_impl &operator=(server_request_impl &&) = delete;
			bool recv_headers();
		};

		server_request_impl::~server_request_impl() {

			// Special case: If the root handler responds with (1) an error status
			// code (4xx or 5xx) and (2) an empty and non-flushed body then fill out
			// the body with the status code reason phrase. This causes error
			// responses to be human-meaningful by default but still allows
			// applications to customize the error response page--even for error
			// responses generated by built-in handlers such as the basic_router.

			if (!sb.headers_written() && denotes_error(rs.status)) {
				auto r = rs.headers.equal_range("content-type");
				rs.headers.erase(r.first, r.second);
#ifdef CLANE_HAVE_STD_MULTIMAP_EMPLACE
				rs.headers.emplace("content-type", "text/plain");
#else
				rs.headers.insert(http::header("content-type", "text/plain"));
#endif
				rs << what(rs.status) << '\n';
			}
		}

		server_request_impl::server_request_impl(server_connection *conn, server_options const *opts):
			sb(conn, opts),
			rs(&sb, sb.out_scode, sb.out_hdrs),
			req(&sb) {}

		bool server_request_impl::recv_headers() {

			while (true) {

				// receive:
				if (!sb.conn->recv_if_none_avail(sb.opts->header_timeout))
					return false;

				// parse:
				size_t pstat = sb.pars.parse_some(sb.conn->icur(), sb.conn->iend());
				if (sb.pars.error == pstat) {
					// FIXME: Send an error response to the client. The parser has the HTTP
					// status code.
					return false;
				}
				sb.conn->ibump(pstat);

				// still incomplete?
				if (!sb.pars.got_headers()) {
					assert(sb.conn->icur() >= sb.conn->iend()); // must have parsed everything if incomplete
					continue; // headers are incomplete--continue receiving
				}

				// Headers are complete: prepare for steam buffering.
				sb.out_major_ver = sb.pars.major_version();
				sb.out_minor_ver = sb.pars.minor_version();
				sb.out_scode = status_code::ok;
				sb.setp(nullptr, nullptr); // force overflow on first write
				sb.in_trls = &req.trailers;
				req.method = std::move(sb.pars.method());
				req.uri = std::move(sb.pars.uri());
				req.major_version = std::move(sb.pars.major_version());
				req.headers = std::move(sb.pars.headers());
				sb.enable();
				return true;
			}
		}

		server_connection::server_connection(net::socket &&conn, net::event &term_ev):
			conn{std::move(conn)},
			term_ev(term_ev),
			icur_{ibuf_},
			iend_{ibuf_}
		{
			this->conn.set_nonblocking();
		}

		bool server_connection::recv_if_none_avail(std::chrono::steady_clock::duration timeout) {

			if (icur() < iend())
				return true; // data already available

			icur_ = iend_ = ibuf_; // reset input buffer
			return recv_some();
		}

		bool server_connection::recv_some(std::chrono::steady_clock::duration timeout) {

			// calculate absolute timeout:
			std::chrono::steady_clock::time_point abs_timeout;
			bool timed{};
			if (std::chrono::steady_clock::duration::zero() != timeout) {
				abs_timeout = std::chrono::steady_clock::now() + timeout;
				timed = true;
			}

			// set up poller:
			net::poller poller;
			size_t const iterm = poller.add(term_ev, poller.in);
			poller.add(conn, poller.in);

			while (true) {

				// wait for incoming data, termination, or timeout:
				auto poll_res = timed ? poller.poll() : poller.poll(abs_timeout);
				if (!poll_res.index)
					return false; // timeout
				if (poll_res.index == iterm)
					return false; // termination

				// receive:
				std::error_code e;
				size_t xstat = conn.recv(ibuf_, sizeof(ibuf_), e);
				if (e == std::errc::operation_would_block || e == std::errc::resource_unavailable_try_again)
					continue; // false positive from the poll operation--continue receiving
				iend_ += xstat;
				if (e)
					return false; // connection error
				if (!xstat)
					return false; // connection FIN
				return true; // data available, connection OK
			}
		}

		bool server_connection::send_all(void const *p, size_t n, std::chrono::steady_clock::duration timeout) {

			// calculate absolute timeout:
			std::chrono::steady_clock::time_point abs_timeout;
			bool timed{};
			if (std::chrono::steady_clock::duration::zero() != timeout) {
				timed = true;
				abs_timeout = std::chrono::steady_clock::now() + timeout;
			}

			// set up poller:
			net::poller poller;
			size_t const iterm = poller.add(term_ev, poller.in);
			poller.add(conn, poller.out);

			// while output buffer is nonempty:
			char const *cur = reinterpret_cast<char const *>(p);
			char const *const end = cur + n;
			while (cur < end) {

				// wait for send readiness, termination, or timeout:
				auto poll_res = timed ? poller.poll() : poller.poll(abs_timeout);
				if (!poll_res.index)
					return false; // timeout
				if (poll_res.index == iterm)
					return false; // termination

				// send:
				std::error_code e;
				size_t xstat = conn.send(cur, end-cur, e);
				if (e == std::errc::operation_would_block || e == std::errc::resource_unavailable_try_again)
					continue; // false positive from the poll operation--continue sending
				if (e)
					return false; // connection error
				if (!xstat)
					return false; // connection FIN
				cur += xstat;
			}
			return true; // all data sent, connection OK
		}

		server_request::~server_request() {}

		server_request::server_request(server_connection *conn, server_options const *opts):
			impl(new server_request_impl(conn, opts)) {}

		bool server_request::recv_headers() {
			return impl->recv_headers();
		}

		http::response_ostream &server_request::response_ostream() {
			return impl->rs;
		}

		http::request &server_request::request() {
			return impl->req;
		}

	}
}

