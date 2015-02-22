// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// vim: set noet:

/** @file */

#ifndef CLANE_CHECK_HXX
#define CLANE_CHECK_HXX

#include <utility>

namespace clane {
	namespace checker {

		class source_stack {
			bool m_top{};
		public:
			~source_stack();
			source_stack(char const *src_filename, unsigned src_line_no);
			source_stack(source_stack const &) = delete;
			source_stack &operator=(source_stack const &) = delete;
			static char const *source_filename();
			static unsigned source_line_number();
		};

		void check_ex(char const *src_filename, unsigned src_line_no, char const *msg, bool cond);

		template <typename Func, typename ...Args> void check_call_ex(char const *src_filename, unsigned src_line_no,
				Func &&f, Args&&... args) {
			source_stack ss{src_filename, src_line_no};
			f(std::forward<Args>(args)...);
		}

	}
}

#define check(cond) \
	do { \
		clane::checker::check_ex(__FILE__, __LINE__, #cond, static_cast<bool>(cond)); \
	} while (false)

#define check_call(f, ...) \
	do { \
		clane::checker::check_call_ex(__FILE__, __LINE__, f, ## __VA_ARGS__); \
	} while (false)

#endif // #ifndef CLANE_CHECK_HXX
