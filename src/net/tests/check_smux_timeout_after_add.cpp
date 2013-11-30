// vim: set noet:

#include "../net.h"
#include "../../check/check.h"
#include <future>
#include <list>

// This test uses a Listener signal because it's the simplest signal currently
// implemented.

static std::condition_variable timeout_cond;
static std::mutex timeout_mutex;
static int timeout_cnt;

class simple_timeout: public clane::net::mux_listener {
public:
	simple_timeout();
	virtual mux_accept_result accept();
	virtual void timed_out();
	void set_timeout(std::chrono::steady_clock::time_point const &timeout);
};

simple_timeout::mux_accept_result simple_timeout::accept() {
	mux_accept_result result{};
	result.aborted = true;
	return result;
}

simple_timeout::simple_timeout(): clane::net::mux_listener(clane::net::listen_tcp("localhost:", 16)) {
}

void simple_timeout::timed_out() {
	set_timeout(std::chrono::steady_clock::now());
	std::lock_guard<std::mutex> lock(timeout_mutex);
	++timeout_cnt;
	timeout_cond.notify_all();
}

void simple_timeout::set_timeout(std::chrono::steady_clock::time_point const &timeout) {
	signal::set_timer(timeout);
}

int main() {

	// run multiplexer:
	clane::net::smux mux;
	std::shared_ptr<simple_timeout> sig(new simple_timeout);
	mux.attach_signal(sig);
	sig->set_timeout(std::chrono::steady_clock::now()); // timeout before mux begins
	auto fut = std::async(std::launch::async, &clane::net::smux::run, &mux);

	// wait for the timeout to occur:
	{
		std::unique_lock<std::mutex> lock(timeout_mutex);
		while (timeout_cnt <= 100)
			timeout_cond.wait(lock);
	}

	// cleanup:
	mux.terminate();
	fut.wait();

	return 0;
}

