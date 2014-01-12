// vim: set noet:

#ifndef CLANE__HTTP_HEADER_HPP
#define CLANE__HTTP_HEADER_HPP

#include "clane_common.hpp"
#include <map>
#include <string>

namespace clane {
	namespace http {

		class header_name_less {
		public:
			bool operator()(std::string const &a, std::string const &b) const;
		};

		typedef std::multimap<std::string, std::string, header_name_less> header_map;
	}
}

#endif // #ifndef CLANE__HTTP_HEADER_HPP
