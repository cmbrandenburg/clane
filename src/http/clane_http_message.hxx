// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// vim: set noet:

#ifndef CLANE_HTTP_MESSAGE_HXX
#define CLANE_HTTP_MESSAGE_HXX

/** @file */

#include "clane_http_message.hpp"
#include "uri/clane_uri.hpp"
#include <cassert>

namespace clane {
	namespace http {

		/** Returns whether a string is a valid HTTP method */
		bool is_method(char const *beg, char const *end);

		/** Returns whether a string is valid HTTP _TEXT_ (as according to RFC 2616) */
		bool is_text(char const *beg, char const *end);

		/** Returns whether the headers specify a chunked transfer coding */
		bool is_chunked(header_map const &hdrs);

		/** Returns whether a string is a valid HTTP chunk size line */
		bool parse_chunk_size(char const *beg, char const *end, std::size_t *osize);

		/** Returns whether the string is a valid HTTP Content-Length value */
		bool parse_content_length(char const *beg, char const *end, std::size_t *olen);

		/** Returns whether a string is a valid HTTP version field */
		bool parse_http_version(char const *beg, char const *end, protocol_version *o_ver);

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

			/** Reinitializes the parser so it may parse again
			 *
			 * @remark Derived classes should overload the reset() function to
			 * reset their derived state, including the call into this base
			 * class implementation. */
			void reset() {
				m_bad = false;
				m_fin = false;
				m_size = 0;
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
			 * parser fully consumes the input buffer `p` or else the parser
			 * stops due to error or success.
			 *
			 * @remark The parse() function works by repeatedly calling the
			 * derived class's `parse_some()` function member. The `parse_some()`
			 * function need only parse at least one byte or otherwise advance its
			 * state. Meanwhile, the parse() function takes care of common chores,
			 * such as checking whether the parser has reached is input-length
			 * capacity or if the parser stopped. */
			std::size_t parse(char const *p, std::size_t n);

		protected:

			/** Sets the parser into a stopped state due to error */
			std::size_t mark_bad(status_code_type stat_code) {
				assert(!bad());
				assert(!fin());
				m_bad = true;
				m_stat_code = stat_code;
				return 0;
			}

			/** Sets the parser into a stopped state due to success */
			void mark_fin() {
				assert(!bad());
				assert(!fin());
				m_fin = true;
			}

			/** Sets the parser into a stopped state due to success */
			std::size_t mark_fin(std::size_t ret) {
				mark_fin();
				return ret;
			}
		};

		/** Base class for HTTP-1.x-style line-oriented parsers
		 *
		 * @remark An HTTP-1.x-style line-oriented parser is a parser that
		 * parses line-by-line, accepting either a CRLF or LF as the line
		 * ending.
		 *
		 * @remark The v1x_line_parser class buffers each line into a contiguous
		 * buffer, as needed, so that derived classes need only implement logic
		 * for handling lines in their entirety. */
		template <typename Derived> class v1x_line_parser: public parser<v1x_line_parser<Derived>> {
			friend class parser<v1x_line_parser<Derived>>;
			std::string m_cur_line;

		public:
			std::size_t constexpr capacity() const {
				return static_cast<Derived const*>(this)->capacity();
			}

			void reset() {
				parser<v1x_line_parser>::reset();
				m_cur_line.clear();
			}

		private:
			std::size_t parse_some(char const *p, std::size_t n);
		};

		/** Parser for an HTTP-1.x-style empty line */
		class v1x_empty_line_parser: public v1x_line_parser<v1x_empty_line_parser> {
			friend class v1x_line_parser;

		public:
			static std::size_t constexpr capacity() { return 2; }

			void reset() {
				v1x_line_parser<v1x_empty_line_parser>::reset();
			}

		private:
			std::size_t parse_line(char const *p, std::size_t n);
		};

		/** Parser for an HTTP-1.1-style chunk-size line */
		class v1x_chunk_size_parser: public v1x_line_parser<v1x_chunk_size_parser> {
			friend class v1x_line_parser;
			std::size_t *m_chunk_size;

		public:

			v1x_chunk_size_parser(std::size_t *o_chunk_size):
				m_chunk_size{o_chunk_size}
			{}

			void reset(std::size_t *o_chunk_size) {
				v1x_line_parser<v1x_chunk_size_parser>::reset();
				m_chunk_size = o_chunk_size;
			}

			static std::size_t constexpr capacity() { return 1024; }

		private:
			std::size_t parse_line(char const *p, std::size_t n);
		};

		/** Parser for an HTTP-1.x-style headers list */
		class v1x_headers_parser: public v1x_line_parser<v1x_headers_parser> {
			friend class v1x_line_parser;
			std::string      m_cur_line;
			header           m_cur_hdr;
			header_map *     m_hdrs;

		public:

			v1x_headers_parser(header_map *o_hdrs):
				m_hdrs{o_hdrs}
			{}

			void reset(header_map *o_hdrs) {
				v1x_line_parser<v1x_headers_parser>::reset();
				m_cur_hdr.clear();
				m_hdrs = o_hdrs;
				m_hdrs->clear();
			}

			static std::size_t constexpr capacity() { return 16384; }

		private:
			std::size_t parse_line(char const *p, std::size_t n);
		};

		/** Parser for an HTTP-1.x-style request line */
		class v1x_request_line_parser: public v1x_line_parser<v1x_request_line_parser> {
			friend class v1x_line_parser;
			typedef uri::uri uri_type;
			std::string *m_method;
			uri_type *m_uri;
			protocol_version *m_ver;

		public:

			v1x_request_line_parser(std::string *o_method, uri_type *o_uri, protocol_version *o_ver):
				m_method{o_method},
				m_uri{o_uri},
				m_ver{o_ver}
			{}

			void reset(std::string *o_method, uri_type *o_uri, protocol_version *o_ver) {
				v1x_line_parser<v1x_request_line_parser>::reset();
				m_method = o_method;
				m_uri = o_uri;
				m_ver = o_ver;
			}

			static std::size_t constexpr capacity() { return 8192; }

		private:
			std::size_t parse_line(char const *p, std::size_t n);
		};
	}
}

#endif // #ifndef CLANE_HTTP_MESSAGE_HXX
