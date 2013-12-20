// vim: set noet:

#include "inc_server.h"

using namespace clane;

void inc_connection::received(char *p, size_t n) {
	line.append(p, n);
	while (line.npos != line.find('\n')) {
		std::istringstream iss(line.substr(0, line.find('\n')+1));
		line.erase(0, line.find('\n')+1);
		int req;
		iss >> req;
		char ch;
		if (!iss || !iss.get(ch) || ch != '\n' || iss.get(ch)) {
			detach(); // hang up
			return;
		}
		std::ostringstream oss;
		oss << req+1 << '\n';
		std::string m = oss.str();
		auto send_result = send(m.data(), m.size(), net::op_all);
		if (net::status::ok != send_result.stat) {
			std::ostringstream ess;
			ess << "non-OK send result: " << net::what(send_result.stat);
			throw std::runtime_error(ess.str());
		}
		if (m.size() != send_result.size) {
			std::ostringstream ess;
			ess << "incomplete send: sent " << send_result.size << " bytes out of " << m.size();
			throw std::runtime_error(ess.str());
		}
	}
}

void inc_connection::finished() {
}

void inc_connection::ialloc() {
	size_t constexpr n = 4096;
	ibuf.reset(new char[n]);
	set_ibuf(ibuf.get(), n);
}

inc_connection::ready_result inc_connection::send_ready() {
	return ready_result::op_complete;
}

std::shared_ptr<net::signal> inc_listener::new_connection(net::socket &&sock) {
	return std::make_shared<inc_connection>(std::move(sock));
}

