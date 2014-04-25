// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// vim: set noet:

#ifndef CLANE__HTTP_CLIENT_HPP
#define CLANE__HTTP_CLIENT_HPP

/** @file */

#include "clane_http_message.hpp"
#include "../clane_base.hpp"
#include "../net/clane_net_event.hpp"
#include "../uri/clane_uri.hpp"
#include <streambuf>
#include <thread>

namespace clane {
	namespace http {

		/** @brief Server-side class for writing an HTTP response message */
		class request_ostream: public std::ostream {
		public:

			std::string &method;
			uri::uri &uri;
			int &major_version;
			int &minor_version;

			/** @brief Headers to send
			 *
			 * @remark Applications may insert zero or more HTTP headers into
			 * the @ref headers member to send those headers as part of the
			 * response message. Setting the @ref headers member has no effect
			 * after at least one byte of the body has been inserted into the
			 * request_ostream instance. */
			header_map &headers;

			/** @brief Trailers to send
			 *
			 * @remark Applications may insert zero or more HTTP trailers into
			 * the @ref trailers member to send those trailers as part of the
			 * request message. */
			header_map &trailers;

		public:
			virtual ~request_ostream() {}
			request_ostream(std::streambuf *sb, std::string &meth, uri::uri &uri, int &major_ver, int &minor_ver,
				   	header_map &hdrs, header_map &trls);
			request_ostream(request_ostream const &) = delete;
			request_ostream(request_ostream &&) = delete;
			request_ostream &operator=(request_ostream const &) = delete;
			request_ostream &operator=(request_ostream &&) = delete;
			void fin(); // XXX: implement
		};

		inline request_ostream::request_ostream(std::streambuf *sb, std::string &meth, uri::uri &uri, int &major_ver,
		int &minor_ver, header_map &hdrs, header_map &trls):
			std::ostream{sb},
			method(meth),
			uri(uri),
			major_version(major_ver),
			minor_version(minor_ver),
			headers(hdrs),
			trailers(trls) {}

		class client_streambuf: public std::streambuf {
			net::socket &sock;
		public:
			std::string out_meth;
			uri::uri out_uri;
			int out_major_ver;
			int out_minor_ver;
			header_map out_hdrs;
			header_map out_trls;
		public:
			virtual ~client_streambuf() {}
			client_streambuf(net::socket &sock): sock(sock) {}
			client_streambuf(client_streambuf const &) = default;
			client_streambuf &operator=(client_streambuf const &) = default;
#ifndef CLANE_HAVE_NO_DEFAULT_MOVE
			client_streambuf(client_streambuf &&) = default;
			client_streambuf &operator=(client_streambuf &&) = default;
#endif
		protected:
			virtual int sync();
			virtual int_type underflow();
			virtual int_type overflow(int_type ch);
		};

		class client_context {
			client_streambuf sb;
			response resp;
			request_ostream rs;
		public:
			~client_context() {}
			client_context(net::socket &sock);
			client_context(client_context const &) = delete;
			client_context &operator=(client_context const &) = delete;
			client_context(client_context &&) = delete;
			client_context &operator=(client_context &&) = delete;
		};

		client_context::client_context(net::socket &sock):
			sb(sock),
			resp{&sb},
			rs{&sb, sb.out_meth, sb.out_uri, sb.out_major_ver, sb.out_minor_ver, sb.out_hdrs, sb.out_trls} {}

		class client_request {
			std::shared_ptr<client_context> ctx;

		public:
			request_ostream &request();
			http::response &response();

			~client_request() {}
			client_request() {}
			client_request(client_request const &) = delete;
			client_request &operator=(client_request const &) = delete;
#ifndef CLANE_HAVE_NO_DEFAULT_MOVE
			client_request(client_request &&) = default;
			client_request &operator=(client_request &&) = default;
#else
			client_request(client_request &&that) noexcept;
			client_request &operator=(client_request &&that) noexcept;
#endif

			void wait(); // XXX: implement
		};

#ifdef CLANE_HAVE_NO_DEFAULT_MOVE

		client_request::client_request(client_request &&that) noexcept {
		}

		client_request &client_request::operator=(client_request &&that) noexcept {
			return *this;
		}

#endif

		/** @brief Stores information for uniquely describing a client connection */
		struct client_target {
			std::string addr;
			// XXX: TLS or unsecure?
		};

		inline bool operator==(client_target const &a, client_target const &b) {
			return a.addr == b.addr;
		}

		inline bool operator!=(client_target const &a, client_target const &b) { return !(a == b); }

		inline bool operator<(client_target const &a, client_target const &b) {
			return a.addr < b.addr;
		}

		inline bool operator<=(client_target const &a, client_target const &b) {
			return a.addr <= b.addr;
		}

		inline bool operator>(client_target const &a, client_target const &b) { return b < a; }
		inline bool operator>=(client_target const &a, client_target const &b) { return b <= a; }

		class client_connection {
		public:
			~client_connection() {}
			client_connection(std::string const &addr); // XXX: implement
			client_connection(client_connection const &) = delete;
			client_connection &operator=(client_connection const &) = delete;
#ifndef CLANE_HAVE_NO_DEFAULT_MOVE
			client_connection(client_connection &&) = default;
			client_connection &operator=(client_connection &&) = default;
#endif
		};

		/** @brief HTTP client */
		class client {
			static size_t const default_max_header_size = 8 * 1024;
			static size_t const default_max_connections = 2;
			size_t max_header_size;
			size_t max_connections;
			std::map<client_target, std::shared_ptr<client_connection>> conns; // shared pointer for copy semantics
			std::mutex mtx;
			std::condition_variable cv;
		public:

			enum class option {
				//pipeline = 1<<0
			};

			typedef int option_set;

			~client() {}
			client();
			client(client const &) = delete;
			client &operator=(client const &) = delete;
#ifndef CLANE_HAVE_NO_DEFAULT_MOVE
			client(client &&) = default;
			client &operator=(client &&) = default;
#else
			client(client &&that) noexcept;
			client &operator=(client &&that) noexcept;
#endif

			client_request new_request(char const *method, uri::uri uri, option_set opts = 0);
		};

		inline client::client():
			max_header_size{default_max_header_size},
			max_connections{default_max_connections} {}

#ifdef CLANE_HAVE_NO_DEFAULT_MOVE

		inline client::client(client &&that) noexcept:
			max_header_size{std::move(that.max_header_size)},
			max_connections{std::move(that.max_connections)} {}

		inline client &client::operator=(client &&that) noexcept {	
			max_header_size = std::move(that.max_header_size);
			max_connections = std::move(that.max_connections);
			return *this;
		}

#endif

	}
}

#endif // #ifndef CLANE__HTTP_CLIENT_HPP
