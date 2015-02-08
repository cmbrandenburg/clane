// vim: set noet:

#include "clane_check.hpp"
#include "../clane_mime.hpp"

using namespace clane;

int main() {

	// parsing /etc/mime.types format:
	mime::map m;
	std::istringstream ss("application/json\t\tjson\n"
			"text/html\t\t\thtml htm shtml\n");
	mime::parse_mime_types(std::inserter(m, m.end()), ss);
	mime::map exp;
	exp[".htm"] = "text/html";
	exp[".html"] = "text/html";
	exp[".json"] = "application/json";
	exp[".shtml"] = "text/html";
	check(m == exp);

	// default table:
	// This will fail if the operating system doesn't provide the mapping. Should
	// Clane provide a minimum set of mappings if the operating system doesn't?
	mime::map::const_iterator p;
	check((p = mime::default_map.find(".html")) != mime::default_map.end() && p->second == "text/html");
}

