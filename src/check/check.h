// vim: set noet:

#ifndef CLANE_CHECK_CHECK_H
#define CLANE_CHECK_CHECK_H

/** @file
 *
 * @brief Unit-testing */

#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>

#define check_true(cond) clane::check::check_true_(__FILE__, __LINE__, cond)
#define check_false(cond) clane::check::check_false_(__FILE__, __LINE__, cond)
#define check_null(p) clane::check::check_null_(__FILE__, __LINE__, p)
#define check_eq(exp, got) clane::check::check_eq_(__FILE__, __LINE__, exp, got)
#define check_cstr_eq(exp, got) clane::check::check_cstr_eq_(__FILE__, __LINE__, exp, got)

#define check_throw(type, expr) \
	do { \
		try { \
			expr; \
			clane::check::fail_throw_(__FILE__, __LINE__, #type, "nothing"); \
		} catch (type &e) { \
		} catch (std::exception &e) { \
			clane::check::fail_throw_(__FILE__, __LINE__, #type, "something else"); \
		} \
	} while (false)
		
namespace clane {
	namespace check {

		void check_true_(char const *sname, int sline, bool cond);

		void check_false_(char const *sname, int sline, bool cond);

		void check_null_(char const *sname, int sline, void const *p);

		template <typename T> void check_eq_(char const *sname, int sline, T const &exp, T const &got);
		void check_eq_(char const *sname, int sline, char const *exp, std::string const &got);

		void check_cstr_eq_(char const *sname, int sline, char const *exp, char const *got);

		void fail_throw_(char const *sname, int sline, char const *exp_type, char const *got);

		template <typename T> void check_eq_(char const *sname, int sline, T const &exp, T const &got) {
			if (!(exp == got)) {
				std::ostringstream ss;
				ss << sname << ":" << sline << ": expected \"" << exp << "\", got \"" << got << "\"\n";
				std::cerr << ss.str();
				std::abort();
			}
		}
	}
}

#endif // #ifndef CLANE_CHECK_CHECK_H
