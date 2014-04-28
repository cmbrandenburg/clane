// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// vim: set noet:

#ifndef CLANE_BASE_HPP
#define CLANE_BASE_HPP

/** @file */

// Every platform-configuration macro definition below denotes a deviation from
// the C++11 standard. The default--i.e., having no macro defined--assumes
// sufficient standard compliance.
//
// New compilers may require additional definitions.

#if !defined DOXYGEN && !defined noexcept && (__GNUC__ == 4 && __GNUC_MINOR__ < 6)
// Partial support for noexcept
#define noexcept
// Fake support for nullptr
#define nullptr 0
// No support for moving arguments into std::thread function
#define CLANE_HAVE_NO_STD_THREAD_MOVE_ARG
#endif

#if !defined DOXYGEN && (__GNUC__ == 4 && __GNUC_MINOR__ < 7)
// Fake support for steady_clock
#define steady_clock monotonic_clock
// No support for default move constructor or move assignment operator
#define CLANE_HAVE_NO_DEFAULT_MOVE
#endif

#endif // #ifndef CLANE_BASE_HPP
