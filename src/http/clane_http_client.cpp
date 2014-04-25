// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// vim: set noet:

/** @file */

#include "clane_http_client.hpp"
#include <cassert>

namespace clane {
	namespace http {

		int client_streambuf::sync() {
			throw std::runtime_error("XXX: implement");
		}

		client_streambuf::int_type client_streambuf::underflow() {
			throw std::runtime_error("XXX: implement");
		}

		client_streambuf::int_type client_streambuf::overflow(int_type ch) {
			throw std::runtime_error("XXX: implement");
		}

		client_request client::new_request(char const *method, uri::uri uri, option_set opts) {

			// TODO: support or HTTPS
			if (uri.scheme != "http")
				throw std::invalid_argument("URI scheme \"" + uri.scheme + "\" is unsupported");


			assert(!opts); // options currently unsupported
			client_target tgt;
			tgt.addr = uri.host + ":" + (uri.port.empty() ? std::string("80") : uri.port);
			// XXX: simpler resolver function
			// XXX: error code for bad lookup
			//auto addr = net::resolve_inet_address(PF_INET, SOCK_STREAM, false, addr_str);

			std::unique_lock<std::mutex> lk(mtx);

			// Use an existing connection if one exists. Otherwise create a new
			// connection.

			auto conn_pos = conns.find(tgt);
			if (conn_pos == conns.end()) {
				std::unique_ptr<client_connection> conn(new client_connection(tgt.addr));
				// XXX: fill out connection
#ifdef CLANE_HAVE_STD_MAP_EMPLACE
				conn_pos = conns.emplace(tgt, std::move(conn)).first;
#else
				conn_pos = conns.insert(std::make_pair(tgt, std::move(conn))).first;
#endif
			}
			//client_connection &conn = *conn_pos->second;

			// Create the request object.

			

			//client_request creq(


			throw std::runtime_error("XXX: implement");
		}

	}
}

