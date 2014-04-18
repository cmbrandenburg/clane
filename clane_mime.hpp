// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// vim: set noet:

#ifndef CLANE_MIME_HPP
#define CLANE_MIME_HPP

/** @file */

#include "clane_base.hpp"
#include <boost/filesystem.hpp>
#include <map>
#include <sstream>

namespace clane {
	namespace mime {

		/** @brief Maps filename extensions to MIME types (e.g., <code>".html"</code>
		 * → <code>"text/html"</code>. */
		typedef std::map<boost::filesystem::path, std::string> map;

		/** @brief Parses an <code>/etc/mime.types</code>-formatted input stream to
		 * generate pairs of MIME extension–type pairs (e.g., <code>{".html",
		 * "text/html"}</code>) */
		template <class OutputIterator, class Istream> void parse_mime_types(OutputIterator o, Istream &is) {

			std::string line, type, ext;
			std::getline(is, line);
			while (is) {

				// erase comment portion in line, if any
				auto p = line.find('#');
				if (p != line.npos)
					line.erase(p);

				// read type and extensions:
				std::istringstream ss(line);
				ss >> type >> ext;
				while (ss) {
					ext.insert(0, ".");
					*o = std::pair<std::string, std::string>(ext, type);
					ss >> ext;
				}

				// advance to next line:
				std::getline(is, line);
			}
		}

		/** @brief Returns a MIME extension–type map containing MIME mappings
		 * provided by the operating system
		 *
		 * @remark Some operating systems provide a database of MIME extension–type
		 * mappings. (For example, most Linux distributions provide
		 * <code>/etc/mime.types</code>, which specifies hundreds of mappings.) The
		 * make_map_from_system() function loads these mappings and returns them as
		 * a map object. */
		map make_map_from_system();

		/** @brief Global MIME extension–type map, populated from the system MIME
		 * mappings
		 *
		 * @sa make_map_from_system() */
		extern map default_map;
	}
}

#endif // #ifndef CLANE_MIME_HPP
