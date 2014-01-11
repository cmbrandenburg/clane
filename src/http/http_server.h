// vim: set noet:

#ifndef CLANE_HTTP_SERVER_H
#define CLANE_HTTP_SERVER_H

/** @file
 *
 * @brief HTTP response status code */

#include "http_common.h"
#include "http_message.h"
#include "../net/net.h"

namespace clane {
	namespace http {

		class server_connection;
		class server_request_streambuf;
		class server_response_streambuf;
		class response_ostream;

		class request_stream: public request, 

		typedef std::function<void(std::unique_ptr<response_ostream> &&, std::unique_ptr<request> &&)> handler;

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
			void operator()(std::unique_ptr<response_ostream> &&resp_strm, std::unique_ptr<request> &&req);
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
			void operator()(std::unique_ptr<response_ostream> &&resp_strm, std::unique_ptr<request> &&req);
		};

		inline void shared_handler::operator()(std::unique_ptr<response_ostream> &&resp_strm, std::unique_ptr<request> &&req) {
			(*h)(std::move(resp_strm), std::move(req));
		}
	}
}

#endif // #ifndef CLANE_HTTP_SERVER_H
