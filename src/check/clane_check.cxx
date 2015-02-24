// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// vim: set noet:

/** @file */

#include "clane_check.hxx"
#include <cstdlib>
#include <iostream>

namespace {
	thread_local char const *g_src_filename{};
	thread_local unsigned g_src_line_no;
}

namespace clane {
	namespace checker {

		source_stack::~source_stack() {
			if (m_top)
				g_src_filename = nullptr;
		}

		source_stack::source_stack(char const *src_filename, unsigned src_line_no) {
			if (!g_src_filename) {
				m_top = true;
				g_src_filename = src_filename;
				g_src_line_no = src_line_no;
			}
		}

		char const *source_stack::source_filename() {
			return g_src_filename;
		}

		unsigned source_stack::source_line_number() {
			return g_src_line_no;
		}

		void check_ex(char const *src_filename, unsigned src_line_no, char const *msg, bool cond) {
			if (!cond) {
				source_stack ss{src_filename, src_line_no};
				if (!cond) {
					std::cerr << source_stack::source_filename() << ":" << source_stack::source_line_number() <<
						": check failed: " << ": " << msg << "\n";
					abort();
				}
			}
		}
	}
}

