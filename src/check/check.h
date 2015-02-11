// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// vim: set noet:

/** @file */

#ifndef CLANE_CHECK_H
#define CLANE_CHECK_H

#include <stdio.h>
#include <stdlib.h>

#define check(c) \
	do { \
		if (!(c)) { \
			fprintf(stderr, "%s:%d: check failed: %s\n", __FILE__, __LINE__, # c); \
			abort(); \
		} \
	} while (0)

#endif // #ifndef CLANE_CHECK_H
