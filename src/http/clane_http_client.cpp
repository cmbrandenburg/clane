// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// vim: set noet:

/** @file */

#include "clane_http_client.hpp"
#include <cassert>

namespace clane {
	namespace http {

		int client_streambuf::sync() {
		}

		client_streambuf::int_type client_streambuf::underflow() {
		}

		client_streambuf::int_type client_streambuf::overflow(int_type ch) {
		}

		client_request client::new_request(char const *method, char const *uri, option_set opts) {
			assert(!opts); // currently unsupported
		}

	}
}

