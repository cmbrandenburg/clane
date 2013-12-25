// vim: set noet:

#ifndef CLANE_HTTP_SERVER_H
#define CLANE_HTTP_SERVER_H

/** @file
 *
 * @brief HTTP response status code */

#include "http_common.h"
#include "http_message.h"
#include "../net/net.h"
#include <istream>
#include <ostream>

namespace clane {
	namespace http {

		class server_connection;
		class server_request_streambuf;
		class server_response_streambuf;
		class irequest;
		class oresponse;

		typedef std::function<void(std::unique_ptr<oresponse> &&resp, std::unique_ptr<irequest> &&req)> handler;

		class shared_handler {
			std::shared_ptr<handler> h;
		public:
			~shared_handler() = default;
			shared_handler(handler const &h): h(new handler(h)) {}
			shared_handler(handler &&h): h(new handler(std::move(h))) {}
			shared_handler(shared_handler const &that) = default;
			shared_handler(shared_handler &&that) = default;
			shared_handler &operator=(shared_handler const &that) = default;
			shared_handler &operator=(shared_handler &&that) = default;
			void operator()(std::unique_ptr<oresponse> &&resp, std::unique_ptr<irequest> &&req);
		};

		class listener: public net::listener {
			shared_handler h;
		public:
			virtual ~listener() noexcept {}
			listener(net::protocol_family const *pf, const char *addr, handler const &h): net::listener(pf, addr), h(h) {}
			listener(net::protocol_family const *pf, std::string addr, handler const &h): net::listener(pf, addr), h(h) {}
			listener(net::protocol_family const *pf, const char *addr, handler &&h): net::listener(pf, addr), h(std::move(h)) {}
			listener(net::protocol_family const *pf, std::string addr, handler &&h): net::listener(pf, addr), h(std::move(h)) {}
			listener(listener &&) = default;
			listener &operator=(listener &&) = default;
		private:
			virtual std::shared_ptr<net::signal> new_connection(net::socket &&sock);
		};

		class server_request_streambuf {
			std::shared_ptr<server_connection> conn;
		public:
			virtual ~server_request_streambuf() = default;
			server_request_streambuf(std::shared_ptr<server_connection> const &conn): conn(conn) {}
			server_request_streambuf(server_request_streambuf const &) = delete;
			server_request_streambuf &operator=(server_request_streambuf &&) = default;
			server_request_streambuf &operator=(server_request_streambuf const &) = delete;
			server_request_streambuf(server_request_streambuf &&) = default;
		};

		class irequest: public request, public std::istream {
			server_request_streambuf strm_buf;
		public:
			virtual ~irequest() = default;
			irequest(std::shared_ptr<server_connection> const &conn, std::unique_ptr<request> &&req);
			irequest(irequest const &) = delete;
			irequest(irequest &&) = default;
			irequest &operator=(irequest const &) = delete;
			irequest &operator=(irequest &&) = default;
		};

		class server_response_streambuf {
			std::shared_ptr<server_connection> conn;
		public:
			virtual ~server_response_streambuf() = default;
			server_response_streambuf(std::shared_ptr<server_connection> const &conn): conn(conn) {}
			server_response_streambuf(server_response_streambuf const &) = delete;
			server_response_streambuf &operator=(server_response_streambuf &&) = default;
			server_response_streambuf &operator=(server_response_streambuf const &) = delete;
			server_response_streambuf(server_response_streambuf &&) = default;
		};

		class oresponse: public std::ostream {
			server_response_streambuf strm_buf;
			status_code stat;
		public:
			virtual ~oresponse() = default;
			oresponse(std::shared_ptr<server_connection> const &conn): strm_buf(conn) {}
			oresponse(oresponse const &) = delete;
			oresponse(oresponse &&) = default;
			oresponse &operator=(oresponse const &) = delete;
			oresponse &operator=(oresponse &&) = default;
			void set_status_code(status_code stat) { this->stat = stat; }
		};

		class error_handler {
			status_code stat;
			std::string what;
		public:
			~error_handler() = default;
			error_handler(status_code stat, char const *what): stat(stat), what(what) {}
			error_handler(status_code stat, std::string const &what): stat(stat), what(what) {}
			error_handler(error_handler const &) = default;
			error_handler(error_handler &&) = default;
			error_handler &operator=(error_handler const &) = default;
			error_handler &operator=(error_handler &&) = default;
			void operator()(std::unique_ptr<oresponse> &&resp, std::unique_ptr<irequest> &&req);
		};

		inline void shared_handler::operator()(std::unique_ptr<oresponse> &&resp, std::unique_ptr<irequest> &&req) {
			(*h)(std::move(resp), std::move(req));
		}

		inline irequest::irequest(std::shared_ptr<server_connection> const &conn, std::unique_ptr<request> &&req):
		 	request(std::move(*req)), strm_buf(conn) {
		}
	}
}

#endif // #ifndef CLANE_HTTP_SERVER_H
