// vim: set noet:

#include "../net.h"
#include "../../check/check.h"

int main() {
	using clane::net::safe_strerror;
	check_eq("Success", safe_strerror(0));
	check_eq("No such file or directory", safe_strerror(ENOENT));
	return 0;
}

