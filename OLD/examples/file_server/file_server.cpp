// vim: set noet:

#include "../../include/clane.hpp"

// file_server
//
//  This example program is a web server that acts as a file server for the
//  current directory. Requests are handled by serving out the file with the
//  same path as the HTTP request's path. For example, a request for
//  "/alpha/bravo.txt" will serve the file "./alpha/bravo.txt", if it exists, or
//  a blank 404 error page if the file doesn't exist.
// 
// Program invocation:
//
//  ./hello          Listen on port 8080 (default port)
//  ./hello :1234    Listen on port 1234
//
int main(int argc, char **argv) {

	char const *saddr = argc == 1 ? ":8080" : argv[1];

	// Set up a server instance.
	auto s = clane::http::make_server(clane::http::file_server("."));
	s.add_listener(saddr);

	// Run the server. This function should never return because this program
	// doesn't call the server instance's terminate() method.
	s.serve();
}

