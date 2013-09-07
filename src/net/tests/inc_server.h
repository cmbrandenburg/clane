// vim: set noet:

#ifndef INC_SERVER_H
#define INC_SERVER_H

#include "../net.h"
#include <sstream>

namespace clane {

	class inc_server_conn: public net::mux_server_conn {
		std::stringstream buf_strm;
	public:
		inc_server_conn(net::socket &&that_sock);
		virtual void finished();
		virtual bool recv_some(char *buf, size_t cap, size_t offset, size_t size);
	};

	class inc_server_listener: public net::mux_listener {
	public:
		inc_server_listener(net::socket &&that_sock);
		virtual mux_accept_result accept();
		std::string local_addr() const;
	};

	void inc_client(int client_no, std::string const &server_addr, int num_requests);
}

#endif // #ifndef INC_SERVER_H
