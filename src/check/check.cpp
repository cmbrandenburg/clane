// vim: set noet:

/** @file */

#include "check.h"
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <sstream>

namespace clane {
	namespace check {

		void check_cstr_eq_(char const *sname, int sline, char const *exp, char const *got) {
			if (!got || strcmp(exp, got)) {
				std::ostringstream ss;
				ss << sname << ":" << sline << ": expected \"" << exp << "\", got " <<
					(got ? "\"" : "") << (got ? got : "nullptr") << (got ? "\"" : "") << "\n";
				std::cerr << ss.str();
				std::abort();
			}
		}

		void check_eq_(char const *sname, int sline, char const *exp, std::string const &got) {
			check_eq_(sname, sline, std::string(exp), got);
		}

		void check_null_(char const *sname, int sline, void const *p) {
			if (p) {
				std::ostringstream ss;
				ss << sname << ":" << sline << ": null pointer check failed\n";
				std::cerr << ss.str();
				std::abort();
			}
		}

		void check_nonnull_(char const *sname, int sline, void const *p) {
			if (!p) {
				std::ostringstream ss;
				ss << sname << ":" << sline << ": non-null pointer check failed\n";
				std::cerr << ss.str();
				std::abort();
			}
		}

		void check_true_(char const *sname, int sline, bool cond) {
			if (!cond) {
				std::ostringstream ss;
				ss << sname << ":" << sline << ": boolean check failed\n";
				std::cerr << ss.str();
				std::abort();
			}
		}

		void check_false_(char const *sname, int sline, bool cond) {
			if (cond) {
				std::ostringstream ss;
				ss << sname << ":" << sline << ": boolean check failed\n";
				std::cerr << ss.str();
				std::abort();
			}
		}

		void fail_throw_(char const *sname, int sline, char const *exp_type, char const *got) {
			std::ostringstream ss;
			ss << sname << ":" << sline << ": expected to throw type " << exp_type << ", got " << got << "\n";
			std::cerr << ss.str();
			std::abort();
		}
	}
}

