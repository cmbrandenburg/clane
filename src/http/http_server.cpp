// vim: set noet:

/** @file */

#include "http_consumer.h"
#include "http_message.h"
#include "http_server.h"

namespace clane {
	namespace http {

		class server_connection: public net::connection {
			std::weak_ptr<server_connection> weak_self;
			enum class phase {
				request_1x
			} cur_phase;
			std::shared_ptr<char> ibuf;
			std::unique_ptr<request> cur_req;
			request_1x_consumer req_1x_cons;
			chunk_line_consumer chunk_line_cons;
			shared_handler h;
		public:
			virtual ~server_connection() noexcept {}
			server_connection(net::socket &&sock, handler const &h);
			server_connection(server_connection &&) = default;
			server_connection &operator=(server_connection &&) = default;
			void set_self(std::shared_ptr<server_connection> const &self) noexcept { weak_self = self; }
		private:
			virtual void received(char *p, size_t n);
			virtual void finished();
			virtual void ialloc();
			virtual void sent();
			void call_handler(handler const &h, std::unique_ptr<oresponse> &&resp, std::unique_ptr<irequest> &&req);
		};

		server_connection::server_connection(net::socket &&sock, handler const &h): net::connection(std::move(sock)),
	 	cur_req(new request), req_1x_cons(*cur_req), h(h) {
			req_1x_cons.set_length_limit(8 * 1024); // max size of request line + headers
		}

		void server_connection::received(char *p, size_t n) {
			size_t offset = 0;
			while (true) {
				switch (cur_phase) {
					case phase::request_1x:
						if (!req_1x_cons.consume(&p[offset], n-offset)) {
							if (!req_1x_cons) {
								auto shared_self = weak_self.lock();
								call_handler(error_handler(req_1x_cons.error_code(), req_1x_cons.what()),
									 	std::unique_ptr<oresponse>(new oresponse(shared_self)),
										std::unique_ptr<irequest>(new irequest(shared_self, std::move(cur_req))));
								return;
							}
						}
						auto content_len = look_up_header<size_t>(cur_req->headers, "content-length");
						auto shared_self = weak_self.lock();
						std::unique_ptr<oresponse> resp(new oresponse(shared_self));
						std::unique_ptr<irequest> req(new irequest(shared_self, std::move(cur_req)));
						if (content_len.stat == content_len.bad_type) {
							call_handler(error_handler(status_code::bad_request, "invalid content length"), std::move(resp), std::move(req));
							return;
						}
						call_handler(h, std::move(resp), std::move(req));
						// TODO: continue parsing
						break;
				}
			}
		}

		void server_connection::finished() {
			// TODO: implement
		}

		void server_connection::ialloc() {
			static constexpr size_t cap = 4096;
			ibuf.reset(new char[cap]);
			set_ibuf(ibuf.get(), cap);
		}

		void server_connection::sent() {
			// TODO: implement
		}

		void server_connection::call_handler(handler const &h, std::unique_ptr<oresponse> &&resp, std::unique_ptr<irequest> &&req) {
			h(std::move(resp), std::move(req));
			// TODO: Send response data.
		}

		std::shared_ptr<net::signal> listener::new_connection(net::socket &&sock) {
			// FIXME: shared copy of handler
			auto conn = std::make_shared<server_connection>(std::move(sock), h);
			conn->set_self(conn);
			return conn;
		}

		void error_handler::operator()(std::unique_ptr<oresponse> &&resp, std::unique_ptr<irequest> &&req) {
			resp->set_status_code(stat);
		}
	}
}

