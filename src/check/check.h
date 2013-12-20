// vim: set noet:

#ifndef CLANE_CHECK_CHECK_H
#define CLANE_CHECK_CHECK_H

/** @file
 *
 * @brief Unit-testing */

#include <cstdlib>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>

#define check_true(cond) clane::check::check_true_(__FILE__, __LINE__, cond)
#define check_false(cond) clane::check::check_false_(__FILE__, __LINE__, cond)
#define check_null(p) clane::check::check_null_(__FILE__, __LINE__, p)
#define check_nonnull(p) clane::check::check_nonnull_(__FILE__, __LINE__, p)
#define check_eq(exp, got) clane::check::check_eq_(__FILE__, __LINE__, exp, got)
#define check_cstr_eq(exp, got) clane::check::check_cstr_eq_(__FILE__, __LINE__, exp, got)
#define check_lt(a, b) clane::check::check_lt_(__FILE__, __LINE__, a, b)
#define check_lteq(a, b) clane::check::check_lteq_(__FILE__, __LINE__, a, b)
#define check_gt(a, b) clane::check::check_gt_(__FILE__, __LINE__, a, b)
#define check_gteq(a, b) clane::check::check_gteq_(__FILE__, __LINE__, a, b)

#define check_throw(type, expr) check_throw_(__FILE__, __LINE__, type, expr)
#define check_throw_(sname, sline, type, expr) \
	do { \
		try { \
			expr; \
			clane::check::fail_throw_(sname, sline, #type, "nothing"); \
		} catch (type &e) { \
		} catch (std::exception &e) { \
			clane::check::fail_throw_(sname, sline, #type, "something else"); \
		} \
	} while (false)
		
namespace clane {
	namespace check {

		void check_true_(char const *sname, int sline, bool cond);
		void check_false_(char const *sname, int sline, bool cond);

		void check_null_(char const *sname, int sline, void const *p);
		template <typename T> void check_null_(char const *sname, int sline, std::unique_ptr<T> const &p) {
		 	check_null_(sname, sline, p.get());
	 	}
		template <typename T> void check_null_(char const *sname, int sline, std::shared_ptr<T> const &p) {
		 	check_null_(sname, sline, p.get());
	 	}

		void check_nonnull_(char const *sname, int sline, void const *p);
		template <typename T> void check_nonnull_(char const *sname, int sline, std::unique_ptr<T> const &p) {
		 	check_nonnull_(sname, sline, p.get());
	 	}
		template <typename T> void check_nonnull_(char const *sname, int sline, std::shared_ptr<T> const &p) {
		 	check_nonnull_(sname, sline, p.get());
	 	}

		void check_eq_(char const *sname, int sline, char const *exp, std::string const &got);
		template <typename T, typename U> void check_eq_(char const *sname, int sline, T const &exp, U const &got) {
			if (!(exp == got)) {
				std::ostringstream ss;
				ss << sname << ":" << sline << ": expected \"" << exp << "\", got \"" << got << "\"\n";
				std::cerr << ss.str();
				std::abort();
			}
		}

		void check_cstr_eq_(char const *sname, int sline, char const *exp, char const *got);

		template <typename T, typename U> void check_lt_(char const *sname, int sline, T const &a, U const &b) {
			if (!(a < b)) {
				std::ostringstream ss;
				ss << sname << ":" << sline << ": expected " << a << " < " << b << ", got otherwise\n";
				std::cerr << ss.str();
				std::abort();
			}
		}

		template <typename T, typename U> void check_lteq_(char const *sname, int sline, T const &a, U const &b) {
			if (!(a <= b)) {
				std::ostringstream ss;
				ss << sname << ":" << sline << ": expected " << a << " <= " << b << ", got otherwise\n";
				std::cerr << ss.str();
				std::abort();
			}
		}

		template <typename T, typename U> void check_gt_(char const *sname, int sline, T const &a, U const &b) {
			if (!(a < b)) {
				std::ostringstream ss;
				ss << sname << ":" << sline << ": expected " << a << " > " << b << ", got otherwise\n";
				std::cerr << ss.str();
				std::abort();
			}
		}

		template <typename T, typename U> void check_gteq_(char const *sname, int sline, T const &a, U const &b) {
			if (!(a <= b)) {
				std::ostringstream ss;
				ss << sname << ":" << sline << ": expected " << a << " >= " << b << ", got otherwise\n";
				std::cerr << ss.str();
				std::abort();
			}
		}

		void fail_throw_(char const *sname, int sline, char const *exp_type, char const *got);
	}
}

#endif // #ifndef CLANE_CHECK_CHECK_H
