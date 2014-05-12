// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// vim: set noet:

/** @file */

#include "clane_net_connbuf.hpp"

namespace clane {
	namespace net {

		connbuf::~connbuf() {}

		connbuf::connbuf(socket &&s, size_t icap, size_t ocap):
			icap(icap),
			ibuf(new char[icap]),
			ocap(ocap),
			obuf(new char[ocap]),
			ce{}
		{
			setg(ibuf.get(), ibuf.get(), ibuf.get());
			setp(obuf.get(), obuf.get()+ocap);

			// Modify socket argument last so that if an exception occurs this
			// function rolls back perfectly.
			s.set_nonblocking();
			this->s = std::move(s); // won't throw
		}
		 
		connbuf::connbuf(connbuf &&that) { swap(that); }

		connbuf &connbuf::operator=(connbuf &&that) { swap(that); return *this; }

		void connbuf::swap(connbuf &that) noexcept {
			std::swap(s, that.s);
			std::swap(icap, that.icap);
			std::swap(ibuf, that.ibuf);
			std::swap(ocap, that.ocap);
			std::swap(obuf, that.obuf);
		}

		int connbuf::send_all() {

			if (pbase() == pptr())
				return 0; // nothing to send--success

			bool const timed = std::chrono::steady_clock::time_point() != wt;

			// TODO: Optimize the special case in which there's no timeout _and_
			// there's no cancellation event by doing a straight send operation
			// without polling.

			// set up poller:
			poller po;
			size_t const icancel = !ce ? 0 : po.add(ce, poller::in);
			po.add(s, poller::in);

			while (true) {

				// wait for incoming data, cancellation, or timeout:
				auto poll_result = timed ? po.poll() : po.poll(wt);
				if (!poll_result.index)
					return traits_type::eof(); // timeout
				if (poll_result.index == icancel)
					return traits_type::eof(); // cancellation

				// send:
				std::error_code e;
				size_t xstat = s.send(pbase(), pptr()-pbase(), all, e);
				if (e == std::errc::operation_would_block || e == std::errc::resource_unavailable_try_again)
					continue; // false positive from the poll operation--try again
				if (e)
					return -1; // connection error
				break; // everything was sent
			}

			setp(obuf.get(), obuf.get()+ocap);
			return 0; // success
		}

		int connbuf::sync() {
			return send_all();
		}

		connbuf::int_type connbuf::overflow(int_type c) {
			if (-1 == send_all())
				return traits_type::eof(); // error
			*pptr() = traits_type::to_char_type(c);
			pbump(1);
			return !traits_type::eof(); // success
		}

		connbuf::int_type connbuf::underflow() {
			if (-1 == recv_some())
				return traits_type::eof(); // error
			if (gptr() == egptr())
				return traits_type::eof(); // connection finished
			return traits_type::to_int_type(*gptr());
		}

	}
}

