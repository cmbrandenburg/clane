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

		server_parser::server_parser(server_transaction *xact) {
			reset(xact);
		}

		void server_parser::reset(server_transaction *xact) {
			m_phase = phase::request_line_method;
			clear_state();
		}

		std::size_t server_parser::parse(char const *p, std::size_t n) {
			throw std::logic_error("TODO: implement");
			return 0;
		}

#if 0 // FIXME
		void serve_error(server_transaction &xact, status_code c) {
			// TODO: implement
			throw std::logic_error("Unimplemented");
		}

		std::size_t request_parser::parse(char const *p, std::size_t n, server_transaction &xact) {
			std::size_t tot=0, len;
			bool complete;

			while (true) {
				switch (m_stat) {

					case state::request_line: {
						if (0 == (len = parse_line(p+tot, n-tot, 8192, m_cur_line, complete))) {
							// If the request line is too long, does it make sense to ever
							// serve a 414 "request URI too long" error instead of the generic
							// "bad request?"
							serve_error(xact, status_code::bad_request);
							return nerror;
						}
						tot += len;
						if (!complete)
							return n;
						if (m_cur_line.empty())
							continue; // allow zero or more empty lines to precede the request line
						auto const sp1 = m_cur_line.find(' ');
						auto const sp2 = m_cur_line.find(' ', sp1+1);
						auto const sp3 = m_cur_line.size();
						std::error_code ec;
						bool valid = sp2 != std::string::npos;
						valid = valid && is_method(&m_cur_line[0], &m_cur_line[sp1]);
						valid = valid && !(xact.m_uri = uri::parse_uri_reference(&m_cur_line[sp1+1], &m_cur_line[sp2], ec)).empty() && !ec;
						valid = valid && parse_http_version(&m_cur_line[sp2+1], &m_cur_line[sp3], xact.m_major_ver, xact.m_minor_ver);
						if (!valid) {
							serve_error(xact, status_code::bad_request);
							return nerror;
						}
						xact.m_method.assign(&m_cur_line[0], &m_cur_line[sp1]);
						m_stat = state::header;
						m_cur_line.clear();
						continue;
					}

					case state::header: {
						if (0 == (len = parse_line(p+tot, n-tot, 8192, m_cur_line, complete))) {
							serve_error(xact, status_code::bad_request);
							return nerror;
						}
						tot += len;
						if (!complete)
							return n;
						if (m_cur_line.empty()) {
							m_stat = state::body;
							m_cur_line.clear();
							continue;
						}

						// TODO: implement
						break;
					}

					case state::body: {
						// TODO: implement
						break;
					}

				}
			}

			return 0; // FIXME
		}

#endif // #if 0

	}
}

