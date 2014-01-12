// vim: set noet:

#include "check.h"
#include "../clane_event.hpp"
#include "../clane_poll.hpp"
#include <thread>

using namespace clane;

int main() {
	net::poller poller;
	net::event ev;
	size_t ev_index = poller.add(ev, poller.in);
	auto res = poller.poll(std::chrono::steady_clock::now());
	check(!res);
	std::thread thrd(&net::event::signal, &ev);
	res = poller.poll();
	thrd.join();
	check(res);
	check(res.index == ev_index);
	check(res.events == poller.in);
}

