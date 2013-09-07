// vim: set noet:

#include "inc_server.h"
#include "../../check/check.h"

namespace clane {
		
	void inc_client(int client_no, std::string const &server_addr, int num_requests)
	{
		// connect to server:
		auto cconn = clane::net::dial_tcp(server_addr);

		// do some requests:
		for (int i = 1; i <= num_requests; ++i) {
			std::stringstream ss;

			// send:
			ss.str("");
			ss << i << "\n";
			auto ostat = cconn.send_all(ss.str().c_str(), ss.str().size());
			check_false(ostat.reset);
			check_eq(ss.str().size(), ostat.size);

			// receive:
			char ibuf[100];
			size_t size = 0;
			while (!size || !memchr(ibuf, '\n', size)) {
				auto istat = cconn.recv(&ibuf[size], sizeof(ibuf) - size - 1);
				check_false(istat.shutdown);
				check_false(istat.reset);
				size += istat.size;
			}
			ibuf[size] = 0; // null terminator
			ss.str(ibuf);
			int got;
			ss >> got;
#if 0
			{
				std::ostringstream tss;
				tss << "client " << client_no << ": recv " << got << "\n";
				std::cerr << tss.str();
			}
#endif
			check_true(ss);
			check_eq(i + 1, got);
		}

		// disconnect from server:
		cconn.close();
	}

	inc_server_conn::inc_server_conn(net::socket &&that_sock): net::mux_server_conn(std::move(that_sock)) {
	}

	void inc_server_conn::finished() {
		mark_for_close(); // reset connection
	}

	bool inc_server_conn::recv_some(char *buf, size_t cap, size_t offset, size_t size) {

		buf_strm.write(&buf[offset], size);
		std::string line;
		std::getline(buf_strm, line);
		while (buf_strm) {

			// undo partial line extraction, wait for more data
			if (buf_strm.eof()) {
				buf_strm.str(line);
				buf_strm.seekp(0, std::ios_base::end);
				break;
			}

			std::istringstream iss(line);
			int n;
			iss >> n;
			if (!iss) {
				std::ostringstream tss;
				tss << "server received invalid line: " << line << "\n";
				std::cerr << tss.str();
				continue; // ignore line
			}

#if 0
			{
				std::ostringstream tss;
				tss << "server: recv " << n << "\n";
				std::cerr << tss.str();
			}
#endif

			std::ostringstream oss;
			oss << n + 1 << "\n";
			send_all(oss.str().c_str(), oss.str().size());
			std::getline(buf_strm, line);
		}
		buf_strm.clear(); // clear eof

		return false; // don't take ownership of buffer
	}

	inc_server_listener::mux_accept_result inc_server_listener::accept() {
		mux_accept_result ret_result{};
		net::accept_result astat = socket().accept();
		if (astat.aborted)
			ret_result.aborted = true;
		else if (astat.conn)
			ret_result.conn.reset(new inc_server_conn(std::move(astat.conn)));
		return ret_result;
	}

	inc_server_listener::inc_server_listener(net::socket &&that_sock): net::mux_listener(std::move(that_sock)) {
	}

	std::string inc_server_listener::local_addr() const {
		return socket().local_addr();
	}
}

