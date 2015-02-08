// vim: set noet:

#include "clane_check.hpp"
#include "../clane_http_file.hpp"
#include <boost/filesystem/fstream.hpp>

using namespace clane;

void create_test_files(boost::filesystem::path const &path) {
	using namespace boost::filesystem;
	ofstream f;
	f.exceptions(f.badbit | f.failbit);
	remove_all(path);
	create_directory(path);
	f.open(path / "alpha.html");
	f << "This is alpha.";
	f.close();
	create_directory(path / "bravo");
	create_directory(path / "bravo/alpha");
	f.open(path / "charlie.js");
	f << "This is charlie.";
	f.close();
	create_directory(path / "/delta");
}

int main() {
	create_test_files("http_serve_file_files");

	// 200 OK
	{
		http::response_record rr;
		std::ostringstream reqss(std::ios_base::in | std::ios_base::out);
		http::request req(reqss.rdbuf());
		http::serve_file(rr.record(), req, "http_serve_file_files/alpha.html");
		check(http::status_code::ok == rr.status);
		http::header_map::const_iterator p;
		check((p = rr.headers.find("content-type")) != rr.headers.end());
		check(p->second == "text/html");
		check((p = rr.headers.find("content-length")) != rr.headers.end());
		check(p->second == "14");
		check(rr.body.str() == "This is alpha.");
	}

	// 404 Not found
	{
		http::response_record rr;
		std::ostringstream reqss(std::ios_base::in | std::ios_base::out);
		http::request req(reqss.rdbuf());
		http::serve_file(rr.record(), req, "http_serve_file_files/doesnt_exist.html");
		check(http::status_code::not_found == rr.status);
	}

	// cleanup:
	boost::filesystem::remove_all("http_serve_file_files");
}

