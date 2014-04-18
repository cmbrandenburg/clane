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
	create_test_files("http_serve_dir_files");

	http::response_record rr;
	std::ostringstream reqss(std::ios_base::in | std::ios_base::out);
	http::request req(reqss.rdbuf());
	http::serve_dir(rr.record(), req, "http_serve_dir_files");
	check(http::status_code::ok == rr.status);
	check(rr.body.str() == "<html><head/><body><pre>\n"
		"<a href=\"alpha.html\">alpha.html</a>\n"
		"<a href=\"bravo/\">bravo/</a>\n"
		"<a href=\"charlie.js\">charlie.js</a>\n"
		"<a href=\"delta/\">delta/</a>\n"
		"</pre></body></html>\n");

	// cleanup:
	boost::filesystem::remove_all("http_serve_dir_files");
}

