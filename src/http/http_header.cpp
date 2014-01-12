// vim: set noet:

#include "http_header.hpp"

namespace clane {
	namespace http {

		bool header_name_less::operator()(std::string const &a, std::string const &b) const {
			size_t const n = std::min(a.size(), b.size());
			for (size_t i = 0; i < n; ++i) {
				char la = std::tolower(a[i]);
				char lb = std::tolower(b[i]);
				if (la < lb)
					return true;
				if (la > lb)
					return false;
					return 1;
			}
			if (n < a.size())
				return false;
			if (n < b.size())
				return true;
			return false;
		}
	}
}

