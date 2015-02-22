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
			 * @remark The parse() function continues parsing input until either the
			 * parser fully consumes the input buffer `p` or else the parser stop due
			 * to either error or success. */
			std::size_t parse(char const *p, std::size_t n) {
				assert(!bad());
				assert(!fin());
				std::size_t size0 = size();
				while (n) {
					std::size_t size1 = size();
					static_cast<Derived*>(this)->parse_some(p, n);
					if (bad())
						return 0;
					if (fin())
						break;
					std::size_t const delta = size()-size1;
					assert(delta);
					p += delta;
					n -= delta;
				}
				return size() - size0;
			}

		protected:

			/** Returns the number of input bytes parsed */
			std::size_t size() const { return m_size; }

			/** Increases the number of input bytes parsed
			 *
			 * @param delta Number of bytes to increase by.
			 *
			 * @param more True if and only if there must be at least one more byte in
			 * addition to those numbered by `delta` in order for parsing to complete
			 * with success.
			 *
			 * @return Returns true if the number of bytes, after addition, is still
			 * within the parser's capacity. */
			bool increase_size(std::size_t delta, bool more) {
				if (m_size + delta + (more?1:0) > static_cast<Derived*>(this)->capacity())
					return false; // error: too big
				m_size += delta;
				return true; // okay
			}

			/** Sets the parser into a stopped state due to error */
			void set_bad(status_code_type stat_code) {
				assert(!bad());
				assert(!fin());
				m_bad = true;
				m_stat_code = stat_code;
			}

			/** Sets the parser into a stopped state due to success */
			void set_fin() {
				assert(!bad());
				assert(!fin());
				m_fin = true;
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
			void parse_some(char const *p, std::size_t n);
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
			void parse_some(char const *p, std::size_t n);
		};
	}
}

#endif // #ifndef CLANE_HTTP_MESSAGE_HXX
