// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// vim: set noet:

/** @file */

#include "clane_check.hxx"
#include <cstdlib>
#include <iostream>
#include <sstream>

namespace {
	thread_local char const *g_src_filename{};
	thread_local unsigned g_src_line_no;
}

namespace clane {
	namespace utest {

		meta::~meta() {
			if (m_top)
				g_src_filename = nullptr;
		}

		meta::meta(char const *src_filename, unsigned src_line_no) {
			if (!g_src_filename) {
				m_top = true;
				g_src_filename = src_filename;
				g_src_line_no = src_line_no;
			}
		}

		char const *meta::source_filename() {
			return g_src_filename;
		}

		unsigned meta::source_line_number() {
			return g_src_line_no;
		}

		void check_ex(char const *src_filename, unsigned src_line_no, char const *msg, bool cond) {
			if (!cond) {
				meta ss{src_filename, src_line_no};
				if (!cond) {
					std::ostringstream ss;
					ss << meta::source_filename() << ":" << meta::source_line_number() <<
						": Check failed: " << ": " << msg << "\n";
					std::cerr << ss.rdbuf();
					abort();
				}
			}
		}
	}
}

