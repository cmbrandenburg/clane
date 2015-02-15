// vim: set noet:

#include "check/check.h"
#include "../clane_uri_impl.hpp"

int main() {
	auto u = clane::uri::parse_uri_reference("http://alpha/bravo/../charlie//delta");
	u.normalize_path();
	check("http://alpha/charlie/delta" == u.string());
}

