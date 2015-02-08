// vim: set noet:

#include "../../include/clane.hpp"

// This handler is invoked for all requests.
void handle(clane::http::response_ostream &rs, clane::http::request &req) {
	rs.headers.insert(clane::http::header("content-type", "text/html"));
	rs << "<html>\n"
		"<head>\n"
		"<meta charset=\"utf-8\">\n"
		"<title>Clane Hello Example</title>\n"
		"</head>\n"
		"<body>\n"
		"<h1>Clane Hello Example</h1>\n"
		"<p>Clane says, &ldquo;Hello!&rdquo;</p>\n"
		"</body>\n"
		"</html>\n";
}

// Program invocation:
//
//  ./hello          Listen on port 8080 (default port)
//  ./hello :1234    Listen on port 1234
//
int main(int argc, char **argv) {

	char const *saddr = argc == 1 ? ":8080" : argv[1];

	// Set up a server instance.
	clane::http::server s;
	s.root_handler = handle;
	s.add_listener(saddr);

	// Run the server. This function should never return because this program
	// doesn't call the server instance's terminate() method.
	s.serve();
}

