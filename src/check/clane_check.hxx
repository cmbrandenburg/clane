// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// vim: set noet:

/** @file */

#ifndef CLANE_CHECK_HXX
#define CLANE_CHECK_HXX

#include <utility>

namespace clane {

	/** Unit test utilities */
	namespace utest {

		class meta {
			bool m_top{};
		public:
			~meta();
			meta(char const *src_filename, unsigned src_line_no);
			meta(meta const &) = delete;
			meta &operator=(meta const &) = delete;
			static char const *source_filename();
			static unsigned source_line_number();
		};

		void check_ex(char const *src_filename, unsigned src_line_no, char const *msg, bool cond);

		template <typename Func, typename ...Args> void check_call_ex(char const *src_filename, unsigned src_line_no,
				Func &&f, Args&&... args) {
			meta m{src_filename, src_line_no};
			f(std::forward<Args>(args)...);
		}

	}
}

#define check(cond) \
	do { \
		clane::utest::check_ex(__FILE__, __LINE__, #cond, static_cast<bool>(cond)); \
	} while (false)

#define check_call(f, ...) \
	do { \
		clane::utest::check_call_ex(__FILE__, __LINE__, f, ## __VA_ARGS__); \
	} while (false)

#endif // #ifndef CLANE_CHECK_HXX
