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

		server_connection::server_connection(net::socket &&conn, net::event &term_ev):
			conn{std::move(conn)},
			term_ev(term_ev),
			icur_{ibuf_},
			iend_{ibuf_},
			obeg_{obuf_},
			ocur_{obuf_}
		{}

		bool server_connection::recv_if_none_avail(std::chrono::steady_clock::duration timeout) {

			if (icur() < iend())
				return true; // data already available

			icur_ = iend_ = ibuf_; // reset input buffer

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

		bool server_connection::send_all(std::chrono::steady_clock::duration timeout) {

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
			poller.add(conn, poller.out);

			// while output buffer is nonempty:
			while (obeg() < ocur()) {

				// wait for send readiness, termination, or timeout:
				auto poll_res = timed ? poller.poll() : poller.poll(abs_timeout);
				if (!poll_res.index)
					return false; // timeout
				if (poll_res.index == iterm)
					return false; // termination

				// send:
				std::error_code e;
				size_t xstat = conn.send(obeg(), ocur()-obeg(), e);
				if (e == std::errc::operation_would_block || e == std::errc::resource_unavailable_try_again)
					continue; // false positive from the poll operation--continue sending
				obeg_ += xstat;
				if (e)
					return false; // connection error
				if (!xstat)
					return false; // connection FIN
			}
			return true; // all data sent, connection OK
		}

		server_streambuf::~server_streambuf() {
			flush(true);
		}

		server_streambuf::server_streambuf(net::socket &&conn, net::event &term_ev, server_options const *sopts):
			conn{std::move(conn)},
			term_ev(term_ev),
			sopts{sopts},
			in_trls{},
			hdrs_written{},
			in_body{},
			ibeg{},
			iend{}
		{
			pars.reset();
			pars.set_length_limit(sopts->max_header_size);
		}

		bool server_streambuf::recv_header() {

			// Here, the connection may be newly accepted, in which case the input
			// buffer is empty. However, the connection may have already received and
			// handled a request, in which case the input buffer may contain leftover
			// data. The data may be an incomplete request or a complete request--or
			// even multiple requests.

			if (in_body) {

				// The previous request handler may not have consumed all body data, so
				// consume any leftover body and trailers now.
				while (pars) {
					if (ibeg == iend && !recv_some())
						return false; // connection error
					size_t pstat = parse_some();
					if (pars.error == pstat)
						return false; // invalid request
					ibeg += pstat;
				}

				// prepare for another request:
				in_body = false;
				hdrs_written = false;
				pars.reset();
				out_hdrs.clear();
			}

			while (true) {

				// receive:
				if (ibeg == iend && !recv_some())
					return false; // connection error

				// parse:
				size_t pstat = parse_some();
				if (pars.error == pstat)
					return false; // invalid request
				ibeg += pstat;
				
				// still incomplete?
				if (!pars.got_headers()) {
					assert(ibeg == iend); // must have parsed everything if incomplete
					continue; // headers are incomplete--continue receiving
				}

				// Headers are complete: prepare for steam buffering.
				in_body = true;
				out_major_ver = pars.major_version();
				out_minor_ver = pars.minor_version();
				out_scode = status_code::ok;
				assert(out_hdrs.empty()); // already cleared
				setp(obuf, obuf); // force overflow on first write
				return true; // headers are complete and available
			}
		}

		bool server_streambuf::recv_some() {
			if (ibeg == iend)
				ibeg = iend = 0;
			while (true) {

				// I/O multiplexing:
				std::chrono::steady_clock::time_point to;
				bool timed{};
				// FIXME: Use either header_timeout or read_timeout, depending on ?
				if (sopts->header_timeout.zero() != sopts->header_timeout) {
					to = std::chrono::steady_clock::now() + sopts->header_timeout;
					timed = true;
				}
				net::poller poller;
				size_t const iterm = poller.add(term_ev, poller.in);
				poller.add(conn, poller.in);

				// wait for incoming data, termination, or timeout:
				auto poll_res = timed ? poller.poll() : poller.poll(to);
				if (!poll_res.index)
					return false; // timeout
				if (poll_res.index == iterm)
					return false; // termination

				// receive:
				std::error_code e;
				size_t xstat = conn.recv(ibuf+iend, sizeof(ibuf)-iend, e);
				if (e == std::errc::operation_would_block || e == std::errc::resource_unavailable_try_again)
					continue; // false positive from the poll operation--continue receiving
				if (e)
					return false; // connection error
				if (!xstat)
					return false; // connection FIN
				iend += xstat;
				return true; // connection okay
			}
		}

		size_t server_streambuf::parse_some() {
			size_t pstat = pars.parse_some(ibuf+ibeg, ibuf+iend);
			if (pars.error == pstat) {
				// FIXME: Send an error response to the client. The parser has the HTTP
				// status code.
			}
			return pstat; // relay result, either success or error
		}

		server_streambuf::int_type server_streambuf::underflow() {

			// XXX: TEST CASE: Root handler doesn't consume all body data and a new
			// request arrives.

			if (gptr() < egptr())
				return traits_type::to_int_type(*gptr());

			// Here, a request handler is trying to get more body data. The input
			// buffer may have leftover data in it.

			while (true) {

				if (ibeg == iend && !recv_some())
					return traits_type::eof(); // connection error

				// parse:
				size_t pstat = parse_some();
				if (pars.error == pstat)
					return traits_type::eof(); // invalid request body
				if (!pars) {
					*in_trls = std::move(pars.trailers());
					if (!pars.size())
						return traits_type::eof(); // no body
				}
				if (pars.size()) {
					setg(ibuf, ibuf+ibeg+pars.offset(), ibuf+ibeg+pars.size());
					ibeg += pstat;
					return traits_type::to_int_type(*gptr()); // success
				}
			}
		}

		int server_streambuf::sync() {
			return flush();
		}

		server_streambuf::int_type server_streambuf::overflow(int_type ch) {
			if (-1 == flush())
				return traits_type::eof();
			setp(obuf, obuf+sizeof(obuf));
			if (traits_type::eof() != ch) {
				*obuf = traits_type::to_char_type(ch);
				pbump(1);
			}
			return !traits_type::eof();
		}

		int server_streambuf::flush(bool end) {

			std::error_code e;

			// flush headers if not already written:
			if (!hdrs_written) {
				chunked = false;
				if (!query_headers_content_length(out_hdrs, content_len)) {
					chunked = true;
#ifdef CLANE_HAVE_STD_MULTIMAP_EMPLACE
					out_hdrs.emplace("transfer-encoding", "chunked");
#else
					out_hdrs.insert(header("transfer-encoding", "chunked"));
#endif
				}
				std::ostringstream ss;
				ss << "HTTP/" << out_major_ver << '.' << out_minor_ver << ' ' << static_cast<int>(out_scode) << ' ' <<
				 	what(out_scode) << "\r\n";
				for (auto i = out_hdrs.begin(); i != out_hdrs.end(); ++i)
					ss << canonize_1x_header_name(i->first) << ": " << i->second << "\r\n";
				ss << "\r\n";
				std::string hdr_lines = ss.str();
				// FIXME: implement send timeout
				conn.send(hdr_lines.data(), hdr_lines.size(), net::all, e);
				if (e)
					return -1; // connection error
				hdrs_written = true;
			}

			// send:
			size_t chunk_len = pptr() - pbase();
			if (chunk_len) {
				if (chunked) {
					std::ostringstream ss;
					ss << std::hex << chunk_len << "\r\n";
					std::string chunk_line = ss.str();
					// FIXME: implement send timeout
					conn.send(chunk_line.data(), chunk_line.size(), net::all, e);
					if (e)
						return -1; // connection error
				} else {
					// Sanity check: Ensure no request handler writes too many content
					// bytes. This may catch some application bugs.
					if (chunk_len > content_len)
						throw std::runtime_error("HTTP request handler wrote too much content");
					content_len -= chunk_len;
				}
				conn.send(pbase(), chunk_len, net::all, e);
				if (e)
					return -1; // connection error
				if (chunked) {
					conn.send("\r\n", 2, net::all, e);
					if (e)
						return -1; // connection error
				}
			}

			// final chunk:
			if (end && chunked) {
				static char const *const final_chunk = "0\r\n\r\n";
				conn.send(final_chunk, std::strlen(final_chunk), net::all, e);
				if (e)
					return -1; // connection error
			} else if (end) {
				// Sanity check: Ensure no request handler writes too few content bytes.
				// This may catch some application bugs.
				if (content_len)
					throw std::runtime_error("HTTP request handler wrote too little content");
			}

			return 0; // success
		}

		void post_handler(server_streambuf &sb, response_ostream &rs, request &req) {

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

	}
}

