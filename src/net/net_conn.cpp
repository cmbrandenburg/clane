// vim: set noet:

/** \file */

#include "net_conn.h"
#include <cstring>

namespace clane {
	namespace net {

		connection::~connection() noexcept {
			while (send_head) {
			 	send_node *next = send_head->next;
			 	delete send_head;
			 	send_head = next;
		 	}
		}

		signal::ready_result connection::read_ready() {
			if (!ibuf)
				ialloc();
			auto recv_result = sock.recv(ibuf+ioffset, icap-ioffset, op_nonblock);
			if (status::would_block == recv_result.stat)
				return ready_result::op_complete;
			if (status::ok != recv_result.stat)
				return ready_result::signal_complete;
			if (!recv_result.size) {
				finished();
				return ready_result::op_complete;
			}
			received(ibuf+ioffset, recv_result.size);
			bool got_all = icap-ioffset == recv_result.size;
			ioffset += recv_result.size;
			if (ioffset == icap)
				ibuf = nullptr;
			return got_all ? ready_result::op_incomplete : ready_result::op_complete;
		}

		signal::ready_result connection::write_ready() {

			if (!connected)
				throw std::logic_error("asynchronous connection operation isn't supported at this time");

			// Send as much from the Send Queue as can be sent without blocking.
			std::unique_lock<std::mutex> send_lock(send_mutex);
			while (send_head) {
				send_lock.unlock();
				bool fin = false;
				if (!send_head->p) {
					sock.send(nullptr, 0, op_fin);
					fin = true;
				} else {
					size_t n = send_head->size - send_head->offset;
					auto send_result = sock.send(send_head->p.get() + send_head->offset, n, op_nonblock);
					if (status::ok == send_result.stat && send_result.size < n) {
						send_lock.lock();
						send_head->offset += send_result.size;
						return ready_result::op_complete; // try again later
					}
					if (status::would_block == send_result.stat)
						return ready_result::op_complete; // try again later
					if (status::ok != send_result.stat)
						return ready_result::signal_complete; // hang up
				}
				send_lock.lock();
				// pop:
				send_node *next = send_head->next;
				delete send_head;
				send_head = next;
				// callback:
				if (!fin)
					sent();
			}
			return ready_result::op_complete; // nothing more to send
		}

		void connection::finish() {
			std::lock_guard<std::mutex> send_lock(send_mutex);
			if (!send_head) {
				sock.send(nullptr, 0, op_fin);
				return;
			}
			send_append(nullptr, 0);
		}

		void connection::send_append(char const *p, size_t n) {
			// new node:
			std::unique_ptr<send_node> node(new send_node);
			if (p) {
				node->p.reset(new char[n]);
				memcpy(node->p.get(), p, n);
				node->offset = 0;
				node->size = n;
			}
			node->next = nullptr;
			// append:
			send_tail->next = node.release();
			send_tail = send_tail->next;
		}

		status connection::send(void const *p, size_t n) {

			size_t offset = 0;

			// First check whether the Send Queue is empty. If it is empty then there
			// are no asynchronous operations in progress and so this operation may
			// proceed.
			std::lock_guard<std::mutex> send_lock(send_mutex);
			if (!send_head) {
				// Normally it's bad to hold a lock while doing an I/O operation, such
				// as the send operation below. However, the send operation must be
				// co-atomic with pushing the buffer into the Send Queue, should the
				// send operation not complete. If the two actions aren't co-atomic then
				// a multiplexer thread may wake up and check the Send Queue before it
				// becomes populated.
				auto send_result = sock.send(p, n, op_nonblock);
				if (status::ok == send_result.stat && n == send_result.size)
					return status::ok; // complete
				if (status::ok != send_result.stat && status::would_block != send_result.stat) {
					detach();
					return send_result.stat; // error
				}
				offset = send_result.size;
			}

			// Don't push an empty buffer. Empty buffers are reserved for sending FIN.
			if (!n)
				return status::in_progress;

			// The send operation didn't complete and must become an asynchronous
			// operation to send whatever unsent data remain. The write_ready method
			// will be called automatically whenever the connection can send more
			// data.
			char const *ppos = reinterpret_cast<char const *>(p);
			send_append(ppos+offset, n-offset);
			return status::in_progress;
		}

		signal::ready_result listener::read_ready() {
			auto accept_result = sock.accept(op_nonblock);
			if (status::would_block == accept_result.stat)
				return ready_result::op_complete;
			if (status::ok != accept_result.stat)
				return ready_result::op_incomplete;
			owner->attach_signal(new_connection(std::move(accept_result.sock)));
			return ready_result::op_incomplete;
		}
	}
}
