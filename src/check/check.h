// vim: set noet:

#ifndef CLANE_CHECK_CHECK_H
#define CLANE_CHECK_CHECK_H

/** @file
 *
 * @brief Unit-testing check macro */

#include <cstdlib>
#include <iostream>
#include <sstream>

/** @brief Assertion macro for unit-testing */
#define check(cond) do { \
	if (!(cond)) { \
		std::ostringstream ss; \
		ss << __FILE__ << ':' << __LINE__ << ": check failure: " << #cond << "\n"; \
		std::cerr << ss.str(); \
		std::abort(); \
	} \
} while (false)

#endif // #ifndef CLANE_CHECK_CHECK_H
