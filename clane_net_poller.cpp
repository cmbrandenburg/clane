// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// vim: set noet:

/** @file */

#include "clane_net_error.hpp"
#include "clane_net_poller.hpp"
#include <unistd.h>

namespace clane {
	namespace net {

		poll_result poller::poll() {
			return poll(-1);
		}

		poll_result poller::poll(std::chrono::steady_clock::time_point const &to) {
			int msecs = std::max(0, static_cast<int>((to - std::chrono::steady_clock::now()) / std::chrono::milliseconds(1)));
			return poll(msecs);
		}

		poll_result poller::poll(std::chrono::steady_clock::duration const &to) {
			int msecs = std::max(0, static_cast<int>(to / std::chrono::milliseconds(1)));
			return poll(msecs);
		}

		poll_result poller::poll(int to) {
			auto res = scan();
			if (res)
				return res;
			int stat = TEMP_FAILURE_RETRY(::poll(items.data(), items.size(), to));
			if (-1 == stat)
				throw std::system_error(errno, os_category(), "poll");
			if (stat)
				res = scan();
			return res;
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

