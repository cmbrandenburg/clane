// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// vim: set noet:

/** @file */

#include "clane/clane_http_server.hpp"
#include "clane_http_message.hxx"
#include "clane_http_server.hxx"

namespace clane {
	namespace http {

		class server_engine::impl {
			std::unique_ptr<impl> m_impl;

			int m_stat;
			enum class phase {
				request_line,
				headers,
				chunk_size,
				chunk_content,
				fixed_length_body,
				indefinite_length_body,
				chunk_empty_line,
				chunk_footers,
				chunk_fin,
			} m_phase;
			status_code_type m_stat_code;
			server_transaction *m_xact;
			union parser_group {
				std::size_t             body_remaining;
				v1x_chunk_size_parser   v1x_chunk_size;
				v1x_empty_line_parser   v1x_empty_line;
				v1x_headers_parser      v1x_headers;
				v1x_request_line_parser v1x_request_line;
				~parser_group() {}
				parser_group() {}
			} m_parser;

		public:

			~impl()
			{
				switch (m_phase) {
					case phase::request_line:
						m_parser.v1x_request_line.~v1x_request_line_parser();
						break;
					case phase::headers:
					case phase::chunk_footers:
						m_parser.v1x_headers.~v1x_headers_parser();
						break;
					case phase::chunk_size:
						m_parser.v1x_chunk_size.~v1x_chunk_size_parser();
						break;
					case phase::chunk_content:
						break; // nothing to do
					case phase::chunk_empty_line:
					case phase::chunk_fin:
						m_parser.v1x_empty_line.~v1x_empty_line_parser();
						break;
					case phase::fixed_length_body:
						break; // nothing to do
					case phase::indefinite_length_body:
						break; // nothing to do
				}

			}

			impl(server_transaction *xact)
			{
				reset(xact);
			}

			void reset(server_transaction *xact) {
				clear_state();
				new (&m_parser.v1x_request_line) v1x_request_line_parser;
				m_phase = phase::request_line;
				m_xact = xact;
			}

			int state() const { return m_stat; }
			void clear_state() { m_stat = 0; }
			status_code_type status_code() const { return m_stat_code; }

			std::size_t parse_some(char const *p, std::size_t n) {

				// Special case: end-of-file. Be explicit and notify the caller to close
				// the connection. Try to serve an error response first--just in case
				// the client has gracefully shut down its sending but can still
				// receive.
				if (!n &&
				    m_phase != phase::chunk_content &&
				    m_phase != phase::fixed_length_body &&
				    m_phase != phase::indefinite_length_body)
				{
					m_stat |= state::close | state::no_handler;
					return 0;
				}

				switch (m_phase) {

					case phase::request_line: {
						auto x = m_parser.v1x_request_line.parse(p, n);
						if (m_parser.v1x_request_line.bad()) {
							m_stat |= state::close | state::no_handler;
							return 0;
						}
						if (m_parser.v1x_request_line.fin()) {
							m_xact->method = std::move(m_parser.v1x_request_line.method());
							m_xact->uri = std::move(m_parser.v1x_request_line.uri());
							m_xact->major_version = m_parser.v1x_request_line.major_version();
							m_xact->minor_version = m_parser.v1x_request_line.minor_version();
							m_parser.v1x_request_line.~v1x_request_line_parser();
							new (&m_parser.v1x_headers) v1x_headers_parser;
							m_phase = phase::headers;
						}
						return x;
					}

					case phase::headers: {
						auto x = m_parser.v1x_headers.parse(p, n);
						if (m_parser.v1x_headers.bad()) {
							m_stat |= state::close | state::no_handler;
							return 0;
						}
						if (m_parser.v1x_headers.fin()) {
							m_xact->request_headers = std::move(m_parser.v1x_headers.headers());
							m_parser.v1x_headers.~v1x_headers_parser();
							header_map::const_iterator pos;
							if (is_chunked(m_xact->request_headers)) {
								new (&m_parser.v1x_chunk_size) v1x_chunk_size_parser;
								m_phase = phase::chunk_size;
							} else if (end(m_xact->request_headers) != (pos = m_xact->request_headers.find("content-length"))) {
								if (!parse_content_length(&pos->second[0], &pos->second[pos->second.size()], &m_parser.body_remaining)) {
									m_stat |= state::close | state::no_handler;
									return 0;
								}
								m_phase = phase::fixed_length_body;
							} else {
								// The body is neither chunked, nor does there exist a
								// Content-Length header. The request body terminates with the
								// connection.
								m_phase = phase::indefinite_length_body;
							}
							m_stat |= state::body_init;
						}
						return x;
					}

					case phase::chunk_size: {
						auto x = m_parser.v1x_chunk_size.parse(p, n);
						if (m_parser.v1x_chunk_size.bad()) {
							m_stat |= state::close | state::no_handler;
							return 0;
						}
						if (m_parser.v1x_chunk_size.fin()) {
							std::size_t len = m_parser.v1x_chunk_size.chunk_size();
							m_parser.v1x_chunk_size.~v1x_chunk_size_parser();
							if (!len) {
								new (&m_parser.v1x_headers) v1x_headers_parser;
								m_phase = phase::chunk_footers;
							} else {
								m_parser.body_remaining = len;
								m_phase = phase::chunk_content;
							}
						}
						return x;
					}

					case phase::chunk_content:
					case phase::fixed_length_body: {
						if (!n) {
							// The body is incomplete. Close the connection, but don't send a
							// response because a request handler may already be in progress.
							// Notify the caller that it should generate an I/O error, if
							// possible.
							m_stat |= state::close | state::body_error;
						}
						auto x = std::min(n, m_parser.body_remaining);
						m_stat |= state::body_more;
						m_xact->more_body(p, x);
						m_parser.body_remaining -= x;
						if (!m_parser.body_remaining) {
							if (phase::chunk_content == m_phase) {
								new (&m_parser.v1x_chunk_size) v1x_chunk_size_parser;
								m_phase = phase::chunk_size;
							} else {
								assert(phase::fixed_length_body == m_phase);
								m_stat |= state::body_eof;
								new (&m_parser.v1x_chunk_size) v1x_request_line_parser;
								m_phase = phase::request_line;
							}
						}
						return x;
					}

					case phase::indefinite_length_body: {
						if (!n) {
							m_stat |= state::body_eof;
						} else {
							m_stat |= state::body_more;
							m_xact->more_body(p, n);
						}
						return n;
					}

					case phase::chunk_empty_line: {
						auto x = m_parser.v1x_empty_line.parse(p, n);
						if (m_parser.v1x_empty_line.bad()) {
							m_stat |= state::close | state::no_handler;
							return 0;
						}
						if (m_parser.v1x_empty_line.fin()) {
							m_parser.v1x_empty_line.~v1x_empty_line_parser();
							new (&m_parser.v1x_chunk_size) v1x_chunk_size_parser;
							m_phase = phase::chunk_size;
						}
						return x;
					}

					case phase::chunk_footers: {
						auto x = m_parser.v1x_headers.parse(p, n);
						if (m_parser.v1x_headers.bad()) {
							m_stat |= state::close | state::no_handler;
							return 0;
						}
						if (m_parser.v1x_headers.fin()) {
							m_xact->request_footers = std::move(m_parser.v1x_headers.headers());
							m_parser.v1x_headers.~v1x_headers_parser();
							new (&m_parser.v1x_empty_line) v1x_empty_line_parser;
							m_phase = phase::chunk_fin;
						}
						return x;
					}

					case phase::chunk_fin: {
						auto x = m_parser.v1x_empty_line.parse(p, n);
						if (m_parser.v1x_empty_line.bad()) {
							m_stat |= state::close | state::no_handler;
							return 0;
						}
						if (m_parser.v1x_empty_line.fin()) {
							m_stat |= state::body_eof;
							m_parser.v1x_empty_line.~v1x_empty_line_parser();
							new (&m_parser.v1x_request_line) v1x_request_line_parser();
							m_phase = phase::request_line;
						}
						return x;
					}
				}

				assert(false);
				return 0;
			}
		};

		server_engine::server_engine(server_transaction *xact):
			m_impl{std::make_unique<server_engine::impl>(xact)}
		{}

		void server_engine::reset(server_transaction *xact) {
			m_impl->reset(xact);
		}

		int server_engine::state() const {
			return m_impl->state();
		}

		void server_engine::clear_state() {
			m_impl->clear_state();
		}

		server_engine::status_code_type server_engine::status_code() {
			return m_impl->status_code();
		}

		std::size_t server_engine::parse_some(char const *p, std::size_t n) {
			return m_impl->parse_some(p, n);
		}

	}
}

