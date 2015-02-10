// vim: set noet:

/** @file */

#ifndef CLANE_HTTP_SERVER_HPP
#define CLANE_HTTP_SERVER_HPP

#include <string>

namespace clane {
	namespace http {

		/** @brief Parses a line, handling both LF and CRLF line endings */
		std::size_t parse_line(char const *p, std::size_t n, std::size_t max, std::string &oline, bool &ocomplete);

	}
}

#endif // #ifndef CLANE_HTTP_SERVER_HPP
