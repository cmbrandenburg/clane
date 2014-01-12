// vim: set noet:

#include "clane_event.hpp"
#include "clane_poll.hpp"
#include "clane_socket.hpp"
#include <sstream>
#include <stdexcept>
#include <unistd.h>

namespace clane {
	namespace net {

		poll_result poller::poll() {
			return poll(-1);
		}

		poll_result poller::poll(std::chrono::steady_clock::time_point const &to) {
			int msecs = std::min(0, static_cast<int>((to - std::chrono::steady_clock::now()) / std::chrono::milliseconds(1)));
			return poll(msecs);
		}

		poll_result poller::poll(std::chrono::steady_clock::duration const &to) {
			int msecs = std::min(0, static_cast<int>(to / std::chrono::milliseconds(1)));
			return poll(msecs);
		}

		poll_result poller::poll(int to) {
			auto res = scan();
			if (res)
				return res;
			int stat = TEMP_FAILURE_RETRY(::poll(items.data(), items.size(), to));
			if (-1 == stat) {
				std::ostringstream ess;
				ess << "poll: " << posix::errno_to_string(errno);
				throw std::runtime_error(ess.str());
			}
			if (!stat)
				return res; // no events
			return scan();
		}

		poll_result poller::scan() {
			poll_result res{};
			for (size_t i = 0; i < items.size(); ++i) {
				if (items[i].revents & POLLNVAL)
					throw std::runtime_error("invalid poll event");
				if (items[i].revents) {
					res.index = i+1;
					res.events = items[i].revents;
					items[i].revents = 0;
					return res;
				}
			}
			return res;
		}
	}
}

