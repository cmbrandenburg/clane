// vim: set noet:

/** @file */

#include "http_header.h"
#include <algorithm>
#include <cctype>

namespace clane {
	namespace http {

		static int header_name_compare(std::string const &a, std::string const &b);

		typedef header_map::value_type header;
		static bool operator==(header const &a, header const &b);
		static bool operator<(header const &a, header const &b);

		int header_name_compare(std::string const &a, std::string const &b) {
			size_t n = std::min(a.size(), b.size());
			for (size_t i = 0; i < n; ++i) {
				char la = std::tolower(a[i]);
				char lb = std::tolower(b[i]);
				if (la < lb)
					return -1;
				if (la > lb)
					return 1;
			}
			if (n < a.size())
				return 1;
			if (n < b.size())
				return -1;
			return 0;
		}

		std::ostream &operator<<(std::ostream &ostrm, header_map const &hdrs) {
			for (auto const &i: hdrs) {
				ostrm << i.first << ": " << i.second << "\n";
			}
			return ostrm;
		}

		bool operator==(header const &a, header const &b) {
			return header_name_compare(a.first, b.first) == 0 && a.second == b.second;
		}

		bool operator<(header const &a, header const &b) {
			int name_result = header_name_compare(a.first, b.first);
			if (name_result < 0)
				return true;
			return name_result == 0 && a.second < b.second;
		}

		bool operator==(header_map const &a, header_map const &b) {
			return a.size() == b.size() &&
				std::equal(a.begin(), a.end(), b.begin(), [](header const &a, header const &b) { return a == b; });
		}

		bool operator<(header_map const &a, header_map const &b) {
			return std::lexicographical_compare(a.begin(), a.end(), b.begin(), b.end(),
					[](header const &a, header const &b) { return a < b; });
		}

		bool operator<=(header_map const &a, header_map const &b) {
			return a == b || a < b;
		}

		bool header_name_less::operator()(std::string const &a, std::string const &b) const {
			return header_name_compare(a, b) < 0;
		}
	}
}

