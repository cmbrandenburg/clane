// vim: set noet:

#ifndef CLANE__HTTP_SERVER_HPP
#define CLANE__HTTP_SERVER_HPP

#include "clane_common.hpp"
#include "../include/clane_http.hpp"
#include <string>

namespace clane {
	namespace http {

		class server {
		public:
			~server() = default;
			server() = default;
			server(server const &) = delete;
			server(server &&) = delete;
			server &operator=(server &&) = delete;
			void add_listener(char const *addr);
			void add_listener(std::string const &addr);
			void run();
			void terminate();
		};
	}
}

#endif // #ifndef CLANE__HTTP_SERVER_HPP
