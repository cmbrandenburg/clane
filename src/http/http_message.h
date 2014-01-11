// vim: set noet:

#ifndef CLANE_HTTP_MESSAGE_H
#define CLANE_HTTP_MESSAGE_H

/** @file
 *
 * @brief HTTP request and response types */

#include "http_common.h"
#include "http_header.h"
#include "http_status.h"
#include "../uri/uri.h"
#include <functional>
#include <istream>
#include <memory>
#include <ostream>
#include <streambuf>

namespace clane {
	namespace http {

		class request {
			std::shared_ptr<std::streambuf> strm_buf;
		public:
			std::string method;
			uri::uri uri;
			int major_version;
			int minor_version;
			header_map headers;
			std::ostream body;
			~request() = default;
			request(std::shared_ptr<std::streambuf> const &sb): strm_buf{sb}, major_version{}, minor_version{}, body{sb.get()} {}
			request(request const &) = default;
			request(request &&) = default;
			request &operator=(request const &) = default;
			request &operator=(request &&) = default;
		};

		class response_ostream: public std::ostream {
			status_code stat;
		public:
			virtual ~response_ostream() = default;
			//response_ostream(std::shared_ptr<server_connection> const &conn): strm_buf(conn) {}
			response_ostream(response_ostream const &) = delete;
			response_ostream(response_ostream &&) = default;
			response_ostream &operator=(response_ostream const &) = delete;
			response_ostream &operator=(response_ostream &&) = default;
			void set_status_code(status_code stat) { this->stat = stat; }
		};
	}
}

#endif // #ifndef CLANE_HTTP_MESSAGE_H
