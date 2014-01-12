// vim: set noet:

#include "../../check/check.h"
#include "../net_posix.hpp"

using namespace clane;

int main() {
	check("Success" == posix::errno_to_string(0));
	check("No such file or directory" == posix::errno_to_string(ENOENT));
}

