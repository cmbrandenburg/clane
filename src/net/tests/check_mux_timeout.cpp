// vim: set noet:

#include "check_mux_timeout.h"
#include "../net_conn.h"
#include "../net_inet.h"
#include "../../check/check.h"
#include <future>

using namespace clane;

class timer_connection: public net::connection {
public:
	virtual ~timer_connection() noexcept = default;
	timer_connection(net::socket &&sock): connection(std::move(sock)) {}
	virtual void received(char *p, size_t n) {}
	virtual void finished() {}
	virtual void ialloc() {}
	virtual void sent() {}
};

class timer_listener: public net::listener {
	int timer_cnt;
	std::mutex mutex;
	std::condition_variable cond;
public:
	virtual ~timer_listener() noexcept = default;
	timer_listener(): listener(net::tcp, "localhost:"), timer_cnt{} {}
	void set_timer_now() { set_timer(std::chrono::steady_clock::now()); }
	void wait_for_timeout();
private:
	virtual void timed_out();
	std::shared_ptr<net::signal> new_connection(net::socket &&sock) { return std::make_shared<timer_connection>(std::move(sock)); }
};
	
void timer_listener::timed_out() {
	{
		std::lock_guard<std::mutex> lock(mutex);
		++timer_cnt;
	}
	cond.notify_all();
}

void timer_listener::wait_for_timeout() {
	std::unique_lock<std::mutex> lock(mutex);
	cond.wait(lock, [&]() { return timer_cnt; });
}

int check_mux_timeout_after_attach(clane::net::mux *mux) {

	// server setup:
	auto server = std::async(std::launch::async, &net::mux::run, mux);
	auto lis = std::make_shared<timer_listener>();
	mux->attach_signal(lis);

	lis->set_timer_now();

	// ensure timeout occurs:
	lis->wait_for_timeout();

	// terminate:
	mux->terminate();
	server.get();
	return 0;
}

int check_mux_timeout_before_attach(clane::net::mux *mux) {

	// server setup:
	auto server = std::async(std::launch::async, &net::mux::run, mux);
	auto lis = std::make_shared<timer_listener>();
	lis->set_timer_now();
	mux->attach_signal(lis);

	// ensure timeout occurs:
	lis->wait_for_timeout();

	// terminate:
	mux->terminate();
	server.get();
	return 0;
}

