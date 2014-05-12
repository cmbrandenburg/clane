// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// vim: set noet:

#ifndef CLANE__NET_CONNBUF_HPP
#define CLANE__NET_CONNBUF_HPP

/** @file */

#include "../clane_base.hpp"
#include "clane_net_event.hpp"
#include "clane_net_socket.hpp"
#include <chrono>
#include <memory>
#include <streambuf>

namespace clane {
	namespace net {

		// XXX: Move to public header.
		// XXX: Document.
		// Note that cancellation events override I/O readiness, and I/O readiness
		// overrides timeouts.
		class connbuf: public std::streambuf {
			net::socket s;
			size_t icap;
			std::unique_ptr<char> ibuf;
			size_t ocap;
			std::unique_ptr<char> obuf;
			std::chrono::steady_clock::time_point rt; // read timeout time
			std::chrono::steady_clock::time_point wt; // read timeout time
			event *ce;                                // cancellation event
		public:
			virtual ~connbuf();
			connbuf(net::socket &&s, size_t icap, size_t ocap);
			connbuf(connbuf const &) = delete;
			connbuf &operator=(connbuf const &) = delete;
#ifndef CLANE_HAVE_NO_DEFAULT_MOVE
			connbuf(connbuf &&) = default;
			connbuf &operator=(connbuf &&) = default;
#else
			connbuf(connbuf &&that) noexcept;
			connbuf &operator=(connbuf &&that) noexcept;
#endif
			void swap(connbuf &that) noexcept;

			net::socket &socket() { return s; }

			void set_read_timeout(std::chrono::steady_clock::time_point t = std::chrono::steady_clock::time_point()) { rt = t; }
			void set_write_timeout(std::chrono::steady_clock::time_point t = std::chrono::steady_clock::time_point()) { wt = t; }
			void set_cancel_event(event *e = nullptr) { ce = e; }

		private:
			virtual int sync();
			virtual int_type overflow(int_type c);
			virtual int_type underflow();
			int send_all();
			int recv_some();
		};

	}
}

#endif // #ifndef CLANE__NET_CONNBUF_HPP
