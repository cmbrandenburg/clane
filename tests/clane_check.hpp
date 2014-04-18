// vim: set noet:

#ifndef CLANE__CHECK_HPP
#define CLANE__CHECK_HPP

/** @file */

#include <cstdlib>
#include <iostream>

/** @brief Assertion macro for unit tests */
#define check(cond) do { \
	if (!(cond)) { \
		std::cerr << __FILE__ << ':' << __LINE__ << ": check failed: " << #cond << '\n'; \
		std::abort(); \
	} \
} while (false)

#endif // #ifndef CLANE__CHECK_HPP
