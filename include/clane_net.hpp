// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// vim: set noet:

#ifndef CLANE_NET_HPP
#define CLANE_NET_HPP

/** @file
 *
 * @brief Low-level networking  */

#include "clane_base.hpp"
#include "clane_posix.hpp"
#include <chrono>
#include <poll.h>
#include <system_error>
#include <vector>

namespace clane {

	namespace net {
	
		union socket_descriptor {
			int n;
			void *p;
		};

		class socket;

		enum {
			all = 1<<0
		};

		struct protocol_family {
			void (*construct_descriptor)(socket_descriptor &sd);
			void (*destruct_descriptor)(socket_descriptor &sd);
			int (*descriptor)(socket_descriptor const &sd);
			socket (*new_listener)(std::string &addr, int backlog);
			socket (*new_connection)(std::string &addr, std::error_code &e);
			void (*set_nonblocking)(socket_descriptor &sd);
			std::string (*local_address)(socket_descriptor &sd);
			std::string (*remote_address)(socket_descriptor &sd);
			socket (*accept)(socket_descriptor &sd, std::string *oaddr, std::error_code &e);
			size_t (*send)(socket_descriptor &sd, void const *p, size_t n, int flags, std::error_code &e);
			size_t (*recv)(socket_descriptor &sd, void *p, size_t n, int flags, std::error_code &e);
			void (*fin)(socket_descriptor &sd);
		};

		class socket {
			protocol_family const *pf;
			socket_descriptor sd;
		public:
			~socket() { if (pf) { pf->destruct_descriptor(sd); }}
			socket() noexcept: pf{} {}
			socket(protocol_family const *pf, clane::posix::unique_fd &&fd): pf{pf} { sd.n = fd.release(); }
			socket(socket const &) = delete;
			socket(socket &&that) noexcept: pf{} { swap(that); }
			socket &operator=(socket const &) = delete;
			socket &operator=(socket &&that) noexcept;
			void swap(socket &that) noexcept;
			int descriptor() const { return pf->descriptor(sd); }
			void set_nonblocking() { return pf->set_nonblocking(sd); }
			std::string local_address() { return pf->local_address(sd); }
			std::string remote_address() { return pf->remote_address(sd); }
			socket accept(std::error_code &e);
			socket accept(std::string &addr_o, std::error_code &e);
			size_t send(void const *p, size_t n, std::error_code &e) { return pf->send(sd, p, n, 0, e); }
			size_t send(void const *p, size_t n, int flags, std::error_code &e) { return pf->send(sd, p, n, flags, e); }
			size_t recv(void *p, size_t n, std::error_code &e) { return pf->recv(sd, p, n, 0, e); }
			size_t recv(void *p, size_t n, int flags, std::error_code &e) { return pf->recv(sd, p, n, flags, e); }
			void fin() { pf->fin(sd); }
		};

		inline socket &socket::operator=(socket &&that) noexcept {
			swap(that);
			return *this;
		}

		inline void socket::swap(socket &that) noexcept {
			std::swap(pf, that.pf);
			std::swap(sd, that.sd);
		}

		inline socket socket::accept(std::error_code &e) {
		 	return pf->accept(sd, nullptr, e);
	 	}

		inline socket socket::accept(std::string &addr_o, std::error_code &e) {
		 	return pf->accept(sd, &addr_o, e);
	 	}

		inline socket listen(protocol_family const *pf, std::string addr, int backlog = -1) {
			return pf->new_listener(addr, backlog);
		}

		inline socket listen(protocol_family const *pf, char const *addr, int backlog = -1) {
			return listen(pf, std::string(addr), backlog);
		}

		inline socket connect(protocol_family const *pf, std::string addr, std::error_code &e) {
			return pf->new_connection(addr, e);
		}

		inline socket connect(protocol_family const *pf, char const *addr, std::error_code &e) {
			return connect(pf, std::string(addr), e);
		}

		extern protocol_family const tcp;
		extern protocol_family const tcp4;
		extern protocol_family const tcp6;

		class event {
			posix::unique_fd fd;
		public:
			~event() = default;
			event();
			event(event const &) = delete;
			event(event &&that) noexcept: fd(std::move(that.fd)) {}
			event &operator=(event const &) = delete;
			event &operator=(event &&that) noexcept { fd = std::move(that.fd); return *this; }
			void signal();
			void reset();
			posix::unique_fd const &descriptor() const { return fd; }
		};

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
			poller(poller &&that) noexcept: items(std::move(that.items)) {}
			poller &operator=(poller const &) = delete;
			poller &operator=(poller &&that) noexcept { items = std::move(that.items); return *this; }
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

#endif // #ifndef CLANE_NET_HPP
