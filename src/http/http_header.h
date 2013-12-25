// vim: set noet:

#ifndef CLANE_HTTP_HEADER_H
#define CLANE_HTTP_HEADER_H

/** @file
 *
 * @brief HTTP header */

#include "http_common.h"
#include <istream>
#include <map>
#include <ostream>
#include <sstream>

namespace clane {
	namespace http {

		class header_name_less {
		public:
			bool operator()(std::string const &a, std::string const &b) const;
		};

		typedef std::multimap<std::string, std::string, header_name_less> header_map;

		bool operator==(header_map const &a, header_map const &b);
		inline bool operator!=(header_map const &a, header_map const &b) { return !(a == b); }
		bool operator<(header_map const &a, header_map const &b);
		bool operator<=(header_map const &a, header_map const &b);
		inline bool operator>(header_map const &a, header_map const &b) { return !(a <= b); }
		inline bool operator>=(header_map const &a, header_map const &b) { return !(a < b); }

		template <class T> struct header_lookup_result {
			enum {
				ok,
				no_exist,
				bad_type
			} stat;
			T value;
		};

		template <class T> header_lookup_result<T> look_up_header(header_map const &hdrs, char const *name) {
			header_lookup_result<T> result{};
			auto pos = hdrs.find(name);
			if (hdrs.end() == pos) {
				result.stat = result.no_exist;
				return result;
			}
			std::istringstream ss(pos->second);
			ss >> result.value;
			if (!ss || !ss.eof()) {
				result.stat = result.bad_type;
				return result;
			}
			return result;
		}
	}
}

#endif // #ifndef CLANE_HTTP_HEADER_H
