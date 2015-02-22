// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// vim: set noet:

#ifndef CLANE_HTTP_MESSAGE_HXX
#define CLANE_HTTP_MESSAGE_HXX

/** @file */

#include <cassert>
#include "clane/clane_http_message.hpp"
#include "clane/clane_uri.hpp"

namespace clane {
	namespace http {

		/** Returns whether a string is a valid HTTP method */
		bool is_method(char const *beg, char const *end);

		/** Returns whether a string is valid HTTP _TEXT_ (as according to RFC 2616) */
		bool is_text(char const *beg, char const *end);

		/** Returns whether a string is a valid HTTP version field */
		bool parse_http_version(char const *beg, char const *end, unsigned &omajor, unsigned &ominor);

		/** Base class for parsers */
		template <typename Derived> struct parser {
			typedef http::status_code status_code_type;
		private:
			bool             m_bad{};
			bool             m_fin{};
			std::size_t      m_size{};
			status_code_type m_stat_code;

		public:

			/** Returns whether the parser has stopped because of an error */
			bool bad() const { return m_bad; }

			/** Returns whether the parser has stopped because of success */
			bool fin() const { return m_fin; }

			/** Returns the HTTP status code representing the parser error, if any */
			status_code_type status_code() const { return m_stat_code; }

			/** Reinitializes the parser so it may parse again */
			void reset() {
				m_bad = false;
				m_fin = false;
				m_size = 0;
				static_cast<Derived*>(this)->reset_derived();
			}

			/** Parses the given buffer
			 *
			 * @param p Input to parse.
			 *
			 * @param n Number of bytes in the input.
			 *
			 * @return On success, number of bytes parsed. On error, zero.
			 *
			 * @remark The parse() function continues parsing input until either the
			 * parser fully consumes the input buffer `p` or else the parser stops
			 * (due to error or success).
			 *
			 * @remark The parse() function works by repeatedly calling the
			 * derived class's `parse_some()` function member. The `parse_some()`
			 * function need only parse at least one byte or otherwise advance its
			 * state. Meanwhile, the parse() function takes care of common chores,
			 * such as checking whether the parser has reached is input-length
			 * capacity or if the parser stopped. */
			std::size_t parse(char const *p, std::size_t n) {
				assert(!bad());
				assert(!fin());
				std::size_t tot = 0;
				while (tot < n) {
					std::size_t const cap = static_cast<Derived*>(this)->capacity();
					std::size_t const len = cap ? std::min(cap-m_size, n-tot) : n-tot;
					std::size_t const x = static_cast<Derived*>(this)->parse_some(p+tot, len);
					if (bad())
						return 0;
					tot += x;
					m_size += x;
					if (fin())
						break;
					if (m_size == cap)
						return set_bad(status_code_type::bad_request);
				}
				return tot;
			}

		protected:

			/** Sets the parser into a stopped state due to error */
			std::size_t set_bad(status_code_type stat_code) {
				assert(!bad());
				assert(!fin());
				m_bad = true;
				m_stat_code = stat_code;
				return 0;
			}

			/** Sets the parser into a stopped state due to success */
			void set_fin() {
				assert(!bad());
				assert(!fin());
				m_fin = true;
			}

			/** Sets the parser into a stopped state due to success */
			std::size_t set_fin(std::size_t ret) {
				set_fin();
				return ret;
			}
		};

		/** Parser for an HTTP-1.x-style headers list */
		class v1x_headers_parser: public parser<v1x_headers_parser> {
			friend class parser;
			std::string      m_cur_line;
			header           m_cur_hdr;
			header_map       m_hdrs;
		public:
			static std::size_t constexpr capacity() { return 16384; }
			header_map &headers() { return m_hdrs; }
		private:
			void reset_derived();
			std::size_t parse_some(char const *p, std::size_t n);
		};

		/** Parser for an HTTP-1.x-style request line */
		class v1x_request_line_parser: public parser<v1x_request_line_parser> {
			friend class parser;
			typedef uri::uri uri_type;
			std::string m_cur_line;
			std::string m_method;
			uri_type m_uri;
			unsigned m_major_ver;
			unsigned m_minor_ver;
		public:
			static std::size_t constexpr capacity() { return 8192; }
		private:
			void reset_derived();
			std::size_t parse_some(char const *p, std::size_t n);
		};
	}
}

#endif // #ifndef CLANE_HTTP_MESSAGE_HXX
