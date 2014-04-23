// vim: set noet:

#include "../../clane_check.hpp"
#include "../clane_sync_wait_group.hpp"

using namespace clane;

int main() {

	{
		sync::wait_group wg;
	}

	{
		sync::wait_group wg;
		auto w1 = wg.new_reference();
	}

	{
		sync::wait_group wg;
		auto w1 = wg.new_reference();
		auto w2 = wg.new_reference();
	}

	// The following will block indefinitely.
	/*
	{
		sync::wait_group wg1;
		auto w1 = wg1.new_reference();
		{
			sync::wait_group wg2;
			auto w2 = wg2.new_reference();
			w1 = wg2.new_reference();
		}
	}
	*/
}



