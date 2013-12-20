// vim: set noet:

#ifndef CLANE_HTTP_HEADER_H
#define CLANE_HTTP_HEADER_H

/** @file
 *
 * @brief HTTP header */

#include "http_common.h"
#include <map>
#include <ostream>

namespace clane {
	namespace http {

		class header_name_less {
		public:
			bool operator()(std::string const &a, std::string const &b) const;
		};

		class header_map: public std::multimap<std::string, std::string, header_name_less> {
		public:
			~header_map() = default;
			header_map() = default;
			header_map(header_map const &) = default;
			header_map(header_map &&) = default;
			header_map &operator=(header_map const &) = default;
			header_map &operator=(header_map &&) = default;
		};

		std::ostream &operator<<(std::ostream &ostrm, header_map const &hdrs);

		bool operator==(header_map const &a, header_map const &b);
		inline bool operator!=(header_map const &a, header_map const &b) { return !(a == b); }
		bool operator<(header_map const &a, header_map const &b);
		bool operator<=(header_map const &a, header_map const &b);
		inline bool operator>(header_map const &a, header_map const &b) { return !(a <= b); }
		inline bool operator>=(header_map const &a, header_map const &b) { return !(a < b); }
	}
}

#endif // #ifndef CLANE_HTTP_HEADER_H
