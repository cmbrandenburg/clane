// vim: set noet:

#include "../net_error.h"
#include "../../check/check.h"

int main() {
	using clane::net::errno_to_string;
	check_eq("Success", errno_to_string(0));
	check_eq("No such file or directory", errno_to_string(ENOENT));
	return 0;
}

