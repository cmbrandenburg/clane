// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// vim: set noet:

#ifndef CLANE__HTTP_SERVER_HPP
#define CLANE__HTTP_SERVER_HPP

/** @file */

#include "clane_http_message.hpp"
#include "../clane_base.hpp"
#include "../net/clane_net_event.hpp"
#include "../net/clane_net_socket.hpp"
#include "../sync/clane_sync_wait_group.hpp"
#include "../../include/clane_http_server.hpp"
#include <sstream>

namespace clane {
	namespace http {

		/** @brief Captures an HTTP response message in order to test a server-side
		 * request handler
		 *
		 * @remark A response_record instance may create a response_ostream
		 * instance whose response data are stored in the response_record
		 * instance. This is useful for testing server-side request handlers. */
		class response_record {
			bool stored;
		public:
			status_code status;
			header_map headers;
			std::istringstream body;
		private:
			std::ostream obody;

		private:

			class recorder {

				class streambuf: public std::streambuf {
					recorder *r;
				public:
					streambuf(recorder *r): r(r) {}
				protected:
					virtual int sync();
					virtual std::streamsize xsputn(char_type const *s, std::streamsize n);
					virtual int_type overflow(int_type ch);
				};

				response_record *rr;
				status_code scode;
				header_map hdrs;
				std::unique_ptr<streambuf> sb;
				std::unique_ptr<response_ostream> rs;
			public:
				~recorder();
				recorder(response_record *rr);
				recorder(recorder const &) = delete;
				recorder &operator=(recorder const &) = delete;
#ifndef CLANE_HAVE_NO_DEFAULT_MOVE
				recorder(recorder &&) = default;
				recorder &operator=(recorder &&) = default;
#else
				recorder(recorder &&that) noexcept: rr{nullptr} { swap(that); }
				recorder &operator=(recorder &&that) noexcept { swap(that); return *this; }
#endif
				void swap(recorder &that) noexcept;
				operator response_ostream &() { return *rs; }
				void store_once();
			};

		public:
			virtual ~response_record() {}
			response_record(): stored{}, body(std::ios_base::in | std::ios_base::out), obody(body.rdbuf()) {}
			response_record(response_record const &) = delete;
			response_record &operator=(response_record const &) = delete;
#ifndef CLANE_HAVE_NO_DEFAULT_MOVE
			response_record(response_record &&) = default;
			response_record &operator=(response_record &&) = default;
#endif
			recorder record() { return recorder(this); }
		};

		inline int response_record::recorder::streambuf::sync() {
			r->store_once();
			return r->rr->obody.flush() ? 0 : -1;
		}

		inline std::streamsize response_record::recorder::streambuf::xsputn(char_type const *s, std::streamsize n) {
			r->store_once();
			return r->rr->obody.write(s, n) ? n : 0; // assume zero bytes written on error
		}

		inline response_record::recorder::streambuf::int_type response_record::recorder::streambuf::overflow(int_type ch) {
			r->store_once();
			if (ch == traits_type::eof())
				return !traits_type::eof();
			return r->rr->obody.put(ch) ? !traits_type::eof() : traits_type::eof();
		}

		inline response_record::recorder::~recorder() {
			if (rr)
				store_once();
	 	}

		inline response_record::recorder::recorder(response_record *rr):
		 	rr(rr), scode(status_code::ok), sb(new streambuf(this)), rs(new response_ostream(sb.get(), scode, hdrs)) {}

		inline void response_record::recorder::swap(recorder &that) noexcept {
			std::swap(rr, that.rr);
			std::swap(scode, that.scode);
			std::swap(sb, that.sb);
			std::swap(rs, that.rs);
		}

		inline void response_record::recorder::store_once() {
			if (rr->stored)
				return;
			rr->stored = true;
			rr->status = scode;
			rr->headers = hdrs; // full copy so handler doesn't see any changes
		}

	}
}

#endif // #ifndef CLANE__HTTP_SERVER_HPP
