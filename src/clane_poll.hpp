// vim: set noet:

#ifndef CLANE__POLL_HPP
#define CLANE__POLL_HPP

#include "clane_common.hpp"
#include <chrono>
#include <poll.h>
#include <vector>

namespace clane {
	namespace net {

		class event;
		class socket;

		class poll_result {
		public:
			size_t index;
			int events;
			operator bool() const { return index; }
		};

		class poller {
		public:
			enum {
				in = POLLIN,
				out = POLLOUT,
				error = POLLERR,
				hangup = POLLHUP
			};
		private:
			std::vector<pollfd> items;
		public:
			~poller() = default;
			poller() = default;
			poller(poller const &) = delete;
			poller(poller &&) = default;
			poller &operator=(poller const &) = delete;
			poller &operator=(poller &&) = default;
			template <class Pollable> size_t add(Pollable const &x, int ev_flags);
			poll_result poll();
			poll_result poll(std::chrono::steady_clock::time_point const &to);
			poll_result poll(std::chrono::steady_clock::duration const &to);
		private:
			poll_result poll(int to);
			poll_result scan();
		};

		template <class Pollable> size_t poller::add(Pollable const &x, int ev_flags) {
			items.push_back(pollfd{x.descriptor(), static_cast<short>(ev_flags), 0});
			return items.size(); // 1-based index
		}
	}
}

#endif // #ifndef CLANE__POLL_HPP
