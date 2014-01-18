// vim: set noet:

#ifndef CLANE_CHECK_CHECK_H
#define CLANE_CHECK_CHECK_H

/** @file
 *
 * @brief Unit-testing check macros */

#include <stdlib.h>

/** @brief Assertion macro for unit-testing */
#define check(cond) do { \
	if (!(cond)) { \
		fprintf(stderr, "%s:%d: check failure: %s\n", __FILE__, __LINE__, #cond); \
		abort(); \
	} \
} while (false)

#endif // #ifndef CLANE_CHECK_CHECK_H
