// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// vim: set noet:

/** @file */

#include "clane_sync_wait_group.hpp"

namespace clane {
	namespace sync {

		wait_group::~wait_group() {
			// wait for the count to become zero:
			std::unique_lock<std::mutex> lock(mutex);
			while (cnt)
				cond.wait(lock);
		}

		wait_group::reference wait_group::new_reference() {
			increment();
			return reference(this);
		};

		void wait_group::decrement() {
			std::lock_guard<std::mutex> lock(mutex);
			--cnt;
			if (0 == cnt)
				cond.notify_one();
		}

		void wait_group::increment() {
			std::lock_guard<std::mutex> lock(mutex);
			++cnt;
		}
	}
}

