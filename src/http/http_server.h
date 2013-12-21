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

		typedef std::function<void(response_ostream &rs, request &req)> handler;

		class listener: public net::listener {
			handler h;
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
	}
}

#endif // #ifndef CLANE_HTTP_SERVER_H
