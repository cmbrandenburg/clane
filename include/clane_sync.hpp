// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// vim: set noet:

#ifndef CLANE_SYNC_HPP
#define CLANE_SYNC_HPP

/** @file */

#include "clane_base.hpp"
#include <condition_variable>
#include <mutex>

namespace clane {

	namespace sync {
	
		/** @brief Object whose destruction blocks until all outstanding references
		 * destruct. */
		class wait_group {
		public:

			/** @brief Sub-reference type */
			class reference {
				wait_group *rc;
			public:

				/** @brief Removes this @ref reference from its wait_group instance */
				~reference() { if (rc) { rc->decrement(); }}

				/** @brief Constructs this @ref reference in the given wait_group
				 * instance */
				reference(wait_group *rc) noexcept: rc{rc} {}

				/** @brief Deleted */
				reference(reference const &) = delete;

				/** @brief Constructs this @ref reference by moving another */
				reference(reference &&that) noexcept: rc{} { swap(that); }

				/** @brief Deleted */
				reference &operator=(reference const &) = delete;

				/** @brief Assigns this @ref reference by assigning another */
				reference &operator=(reference &&that) noexcept { swap(that); return *this; }

				/** @brief Swaps this @ref reference with another */
				void swap(reference &that) noexcept { std::swap(rc, that.rc); }
			};

		private:
			int cnt;
			std::mutex mutex;
			std::condition_variable cond;

		public:

			/** @brief Blocks until this wait_group becomes empty */
			~wait_group();

			/** @brief Construct this wait_group as empty */
			wait_group(): cnt{} {}

			/** @brief Deleted */
			wait_group(wait_group const &) = delete;

			/** @brief Deleted */
			wait_group(wait_group &&) = delete;

			/** @brief Deleted */
			wait_group &operator=(wait_group const &) = delete;

			/** @brief Deleted */
			wait_group &operator=(wait_group &&) = delete;

			/** @brief Creates a new reference in this wait_group */
			reference new_reference(); // return a new reference whose destruction is waited on

		private:
			void decrement();
			void increment();
		};

	}

}

#endif // #ifndef CLANE_SYNC_HPP
