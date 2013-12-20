// vim: set noet:

/** \file */

#include "net_conn.h"

namespace clane {
	namespace net {

		signal::ready_result connection::read_ready() {
			if (!ibuf)
				ialloc();
			auto recv_result = sock.recv(ibuf+ioffset, icap-ioffset, op_nonblock);
			if (status::would_block == recv_result.stat)
				return ready_result::op_complete;
			if (status::ok != recv_result.stat) {
				detach();
				return ready_result::op_complete;
			}
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
			if (connected) {
				return send_ready();
			}
			throw std::logic_error("asynchronous connection operation isn't supported at this time");
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
