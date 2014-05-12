// vim: set noet:

#include "../../clane_check.hpp"
#include "../clane_net_connbuf.hpp"

using namespace clane;

int main() {

	std::error_code e;
	char buf[100];
	size_t xstat;
	std::string str;

	// connect & accept:
	net::socket ssock, csock;
	{
		auto lsock = net::listen(&net::tcp, "localhost:");
		csock = net::connect(&net::tcp, lsock.local_address(), e);
		check(!e);
		std::string accept_addr;
		ssock = lsock.accept(accept_addr, e);
		check(!e);
	}

	// set up connection buffers with I/O streams:
	net::connbuf s(std::move(ssock), 4096, 4096);
	std::iostream ss(&s);
	net::connbuf c(std::move(csock), 4096, 4096);
	std::iostream cs(&c);

	// output is buffered:
	cs << "alpha bravo\n";
	xstat = c.socket().recv(buf, sizeof(buf), e);
	check(e == std::errc::operation_would_block || e == std::errc::resource_unavailable_try_again);
	cs.flush();
	check((ss >> str) && "alpha" == str);
	check((ss >> str) && "bravo" == str);

	// read timeout:
	s.set_read_timeout(std::chrono::steady_clock::now());
	check(!(ss >> str));
	ss.clear();

	// read cancellation:
	s.set_read_timeout();
	net::event ce;
	s.set_cancel_event(&ce);
	ce.signal();
	check(!(ss >> str));
	ss.clear();

	// send cancellation:
	// Cancellation overrides write readiness.
	check(!(ss << "charlie\n").flush());
	ss.clear();

	// send timeout:
	// Write readiness overrides timeout.
	s.set_cancel_event();
	s.set_write_timeout(std::chrono::steady_clock::now());
	int iout = 1;
	while (ss << (iout++) << '\n');
	ss.clear();
	s.set_write_timeout();
	check(ss.flush());
	//std::cout << "iout: " << iout << '\n';

	// read everything in client:
	// Cancellation overrides read readiness.
	c.set_cancel_event(&ce);
	check(!(cs >> str));	
	cs.clear();
	c.set_cancel_event();
	// Read readiness overrides timeout.
	check((cs >> str) && "charlie" == str);
	int iin = 1;
	int v;
	c.set_read_timeout(std::chrono::steady_clock::now());
	while (iin < iout && (cs >> v) && v == iin++);
	//std::cout << "iin: " << iin << '\n';
	check(!(cs >> v)); // should time out
	check(iin == iout);

	return 0;
}

