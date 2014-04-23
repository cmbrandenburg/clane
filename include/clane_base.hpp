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
#define noexcept // partial support for noexcept
#endif

#if !defined DOXYGEN && !defined nullptr && (__GNUC__ == 4 && __GNUC_MINOR__ < 6)
#define nullptr 0 // fake support for nullptr
#endif

#if !defined DOXYGEN && !defined steady_clock && (__GNUC__ == 4 && __GNUC_MINOR__ < 7)
#define steady_clock monotonic_clock // fake support for steady_clock
#endif

#if !defined DOXYGEN && (__GNUC__ == 4 && __GNUC_MINOR__ < 7)
// No support for default move constructor or move assignment operator
#define CLANE_HAVE_NO_DEFAULT_MOVE
#endif

#endif // #ifndef CLANE_BASE_HPP
