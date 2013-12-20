// vim: set noet:

#include "check_mux_n_conns_n_threads.h"
#include "../net_inet.h"
#include "../../check/check.h"
#include "inc_server.h"
#include <cstring>
#include <deque>
#include <future>
#include <iostream> // FIXME

using namespace clane;

void client_main(net::protocol_family const *pf, std::string saddr, int nreqs) {
	net::socket sock(pf, saddr);
	std::string line;
	for (int i = 0; i < 2*nreqs; i+=2) {
		// send:
		std::stringstream oss;
		oss.exceptions(oss.badbit);
		oss << i << "\n";
		std::string m = oss.str();
		auto send_result = sock.send(m.data(), m.size(), net::op_all);
		if (net::status::ok != send_result.stat) {
			std::ostringstream ess;
			ess << "non-OK send result: " << net::what(send_result.stat);
			throw std::runtime_error(ess.str());
		}
		if (m.size() != send_result.size) {
			std::ostringstream ess;
			ess << "incomplete send: sent " << send_result.size << " bytes of out " << m.size();
			throw std::runtime_error(ess.str());
		}
		// receive:
		do {
			char buf[100];
			auto recv_result = sock.recv(buf, sizeof(buf));
			if (net::status::ok != recv_result.stat) {
				std::ostringstream ess;
				ess << "non-OK receive result: " << net::what(recv_result.stat);
				throw std::runtime_error(ess.str());
			}
			if (!recv_result.size)
				throw std::runtime_error("server hangup");
			line.append(buf, recv_result.size);
		} while (line.npos == line.find('\n'));
		std::istringstream iss(line);
		line.clear();
		int resp;
		iss >> resp;
		char ch;
		if (!iss || !iss.get(ch) || ch != '\n' || iss.eof()) {
			std::ostringstream ess;
			ess << "received unexpected content \"" << line << '"';
			throw std::runtime_error(ess.str());
		}
		if (resp != i + 1) {
			std::ostringstream ess;
			ess << "expected " << i+1 << ", got " << resp;
			throw std::runtime_error(ess.str());
		}
		//std::cout << resp << "\n";
	}
}

int check_mux_n_conns_n_threads(net::mux *mux, int nconns, int nthreads) {

	// server setup:
	std::deque<std::future<void>> servers;
	for (int i = 0; i < nthreads; ++i)
		servers.push_back(std::async(std::launch::async, &net::mux::run, mux));
	auto lis = std::make_shared<inc_listener>(net::tcp, "localhost:");
	mux->attach_signal(lis);

	// client setup:
	std::deque<std::future<void>> clients;
	for (int i = 0; i < nconns; ++i)
		clients.push_back(std::async(std::launch::async, client_main, lis->protocol(), lis->address(), 1000));

	// shutdown:
	while (!clients.empty()) {
		clients.front().get();
		clients.pop_front();
	}
	mux->terminate();
	while (!servers.empty()) {
		servers.front().get();
		servers.pop_front();
	}
	return 0;
}

