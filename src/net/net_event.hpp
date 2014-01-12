// vim: set noet:

#ifndef CLANE__NET_EVENT_HPP
#define CLANE__NET_EVENT_HPP

#include "net_common.hpp"
#include "net_posix.hpp"

namespace clane {
	namespace net {

		class event {
			posix::file_descriptor fd;
		public:
			~event() = default;
			event();
			event(event const &) = delete;
			event(event &&) = default;
			event &operator=(event const &) = delete;
			event &operator=(event &&) = default;
			void signal();
			void reset();
			posix::file_descriptor const &descriptor() const { return fd; }
		};
	}
}

#endif // #ifndef CLANE__NET_EVENT_HPP
