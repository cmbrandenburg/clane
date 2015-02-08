// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// vim: set noet:

#ifndef CLANE_HPP
#define CLANE_HPP

#include "clane_ascii_pub.hpp"
#include "clane_base_pub.hpp"
#include "clane_http_file.hpp"
#include "clane_http_prefix_stripper.hpp"
#include "clane_http_pub.hpp"
#include "clane_http_route.hpp"
#include "clane_net_pub.hpp"
#include "clane_posix_pub.hpp"
#include "clane_regex.hpp"
#include "clane_sync_pub.hpp"
#include "clane_uri_pub.hpp"

/** @file
 *
 * @brief Master header
 *
 * @remark This header file <code>\#include</code>s all other public header
 * files for @projectname. Applications should include only this one header
 * file. */

/** @brief Root namespace for @projectname */
namespace clane {

	/** @brief ASCII string operations */
	namespace ascii {}
	
	/** @brief Hypertext Transfer Protocol */
	namespace http {}

	/** @brief Multipurpose Internet Mail Extensions */
	namespace mime {}

	/** @brief Low-level networking */
	namespace net {}

	/** @brief POSIX utilities */
	namespace posix {}

	/** @brief Regular expression support */
	namespace regex {}

	/** @brief Concurrency synchronization */
	namespace sync {}

	/** @brief Uniform Resource Identifier */
	namespace uri {}

}

#endif // #ifndef CLANE_HPP
