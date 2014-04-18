// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// vim: set noet:

/** @file */

#include "clane_http_parse.hpp"
#include "clane_http_server.hpp"
#include "clane_net_inet.hpp"
#include "clane_net_poller.hpp"

namespace clane {
	namespace http {

		// FIXME: What to do with default_error_handler? Remove?
		void default_error_handler(response_ostream &rs, request &req, status_code stat, std::string const &msg) {
			rs.status = stat;
#ifdef CLANE_HAVE_STD_MULTIMAP_EMPLACE
			rs.headers.emplace("content-type", "text/plain");
#else
			rs.headers.insert(header("content-type", "text/plain"));
#endif
			rs << static_cast<int>(stat) << ' ' << what(stat) << '\n';
			if (!msg.empty())
				rs << msg << '\n';
		}

		server_streambuf::~server_streambuf() {
			if (enabled)
				flush(true);
		}

		server_streambuf::server_streambuf(net::socket &sock): sock(sock), major_ver{}, minor_ver{},
		 	out_stat_code(status_code::ok), in_end{}, enabled{}, active{true}, hdrs_written{}, chunked{} {
			in_queue.push_back(buffer{}); // dummy node
			setp(out_buf, out_buf); // force overflow on first write
		}

		void server_streambuf::more_request_body(std::shared_ptr<char> const &p, size_t offset, size_t size) {
			if (!size)
				return;
			std::lock_guard<std::mutex> in_lock(in_mutex);
			bool empty = in_queue.empty();
			if (!empty && in_queue.back().p.get() + in_queue.back().size == p.get() + offset) {
				// Special case: continuation of previous buffer. Append the new buffer
				// to the last buffer in the queue.
				in_queue.back().size += size;
			} else {
				in_queue.push_back(buffer{std::shared_ptr<char>(p, p.get()+offset), size});
			}
			if (empty)
				in_cond.notify_one();
		}

		void server_streambuf::end_request_body() {
			std::lock_guard<std::mutex> in_lock(in_mutex);
			in_end = true;
			if (in_queue.empty())
				in_cond.notify_one();
		}

		void server_streambuf::inactivate() {
			std::lock_guard<std::mutex> out_lock(act_mutex);
			active = false;
		}

		void server_streambuf::activate() {
			std::lock_guard<std::mutex> out_lock(act_mutex);
			active = true;
			act_cond.notify_one();
		}

		int server_streambuf::flush(bool end) {
			std::error_code e;
			// wait for previous responses in the pipeline to complete:
			{
				std::unique_lock<std::mutex> out_lock(act_mutex);
				while (!active)
					act_cond.wait(out_lock);
			}
			// flush headers if not already written:
			if (!hdrs_written) {
				size_t content_len;
				if (!query_headers_content_length(out_hdrs, content_len)) {
					chunked = true;
#ifdef CLANE_HAVE_STD_MULTIMAP_EMPLACE
					out_hdrs.emplace("transfer-encoding", "chunked");
#else
					out_hdrs.insert(header("transfer-encoding", "chunked"));
#endif
				}
				std::ostringstream ss;
				ss << "HTTP/" << major_ver << '.' << minor_ver << ' ' << static_cast<int>(out_stat_code) << ' ' <<
				 	what(out_stat_code) << "\r\n";
				for (auto i = out_hdrs.begin(); i != out_hdrs.end(); ++i)
					ss << canonize_1x_header_name(i->first) << ": " << i->second << "\r\n";
				ss << "\r\n";
				std::string hdr_lines = ss.str();
				sock.send(hdr_lines.data(), hdr_lines.size(), net::all, e);
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
					sock.send(chunk_line.data(), chunk_line.size(), net::all, e);
					if (e)
						return -1; // connection error
				}
				sock.send(pbase(), chunk_len, net::all, e);
				if (e)
					return -1; // connection error
				if (chunked) {
					sock.send("\r\n", 2, net::all, e);
					if (e)
						return -1; // connection error
				}
			}
			// final chunk:
			if (end && chunked) {
				sock.send("0\r\n\r\n", 5, net::all, e);
				if (e)
					return -1; // connection error
			}
			return 0; // success
		}

		int server_streambuf::sync() {
			return flush();
		}

		server_streambuf::int_type server_streambuf::underflow() {
			std::unique_lock<std::mutex> in_lock(in_mutex);
			in_queue.pop_front();
			while (in_queue.empty() && !in_end)
				in_cond.wait(in_lock);
			if (in_end)
				return traits_type::eof();
			buffer const &b = in_queue.front();
			setg(b.p.get(), b.p.get(), b.p.get()+b.size);
			return traits_type::to_int_type(*b.p);
		}

		server_streambuf::int_type server_streambuf::overflow(int_type ch) {
			if (-1 == flush())
				return traits_type::eof();
			setp(out_buf, out_buf+sizeof(out_buf));
			if (traits_type::eof() != ch) {
				*out_buf = traits_type::to_char_type(ch);
				pbump(1);
			}
			return !traits_type::eof();
		}

		server_context::~server_context() {
			std::lock_guard<std::mutex> next_lock(next_mutex);
			// Invariant: This context is active.
			if (next_ctx)
				next_ctx->sb.activate();
		}

		void server_context::set_next_context(std::shared_ptr<server_context> const &nc) {
			{
				std::lock_guard<std::mutex> next_lock(next_mutex);
				next_ctx = nc;
			}
			nc->sb.inactivate();
		}

	}
}

