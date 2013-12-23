// vim: set noet:

#ifndef CLANE_NET_TESTS_INC_SERVER_H
#define CLANE_NET_TESTS_INC_SERVER_H

#include "../net_conn.h"
#include <sstream>

class inc_connection: public clane::net::connection {
	std::unique_ptr<char[]> ibuf;
	std::string line;
public:
	virtual ~inc_connection() noexcept {}
	inc_connection(clane::net::socket &&sock): connection(std::move(sock)) {}
private:
	virtual void received(char *p, size_t n);
	virtual void finished() {}
	virtual void ialloc();
	virtual void sent() {}
};

class inc_listener: public clane::net::listener {
public:
	inc_listener(clane::net::protocol_family const *pf, char const *addr): listener(pf, addr) {}	
	inc_listener(inc_listener &&) = default;
	inc_listener &operator=(inc_listener &&) = default;
private:
	virtual std::shared_ptr<clane::net::signal> new_connection(clane::net::socket &&sock);
};

#endif // #ifndef CLANE_NET_TESTS_INC_SERVER_H
