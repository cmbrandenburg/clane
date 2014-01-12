// vim: set noet:

#ifndef CLANE__EVENT_HPP
#define CLANE__EVENT_HPP

#include "clane_common.hpp"
#include "clane_posix.hpp"

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

#endif // #ifndef CLANE__EVENT_HPP
