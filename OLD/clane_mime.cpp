// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// vim: set noet:

/** @file */

#include "clane_mime.hpp"
#include <fstream>

namespace clane {
	namespace mime {

		map default_map = make_map_from_system();

		map make_map_from_system() {
			map m;
			std::ifstream in("/etc/mime.types");
			parse_mime_types(std::inserter(m, m.end()), in);
			return m;
		}
	}
}

