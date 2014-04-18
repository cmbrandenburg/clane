// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// vim: set noet:

/** @file */

#include "clane_http_file.hpp"
#include "clane_mime.hpp"
#include <boost/filesystem/fstream.hpp>

namespace clane {
	namespace http {

		void serve_dir(response_ostream &rs, request &req, boost::filesystem::path const &path) {

			// sort directory entries:
			std::map<std::string, bool> ents; // path -> is directory?
			boost::filesystem::directory_iterator i(path), end_iter;
			while (i != end_iter) {
				ents[i->path().filename().string()] = boost::filesystem::directory_file == i->status().type();
				++i;
			}

			// output:
#ifdef CLANE_HAVE_STD_MULTIMAP_EMPLACE
			rs.headers.emplace("content-type", "text/html; charset=utf-8");
#else
			rs.headers.insert(header("content-type", "text/html; charset=utf-8"));
#endif
			// FIXME: "Date"
			// FIXME: "Last-Modified"
			// FIXME: no caching
			rs << "<html><head/><body><pre>\n";
			for (auto &&i = ents.cbegin(); i != ents.cend(); ++i) {
				rs << "<a href=\"" << i->first << (i->second ? "/" : "") << "\">" << i->first << (i->second ? "/" : "") << "</a>\n";
			}
			rs << "</pre></body></html>\n";
		}

		void serve_file(response_ostream &rs, request &req, boost::filesystem::path const &path,
			 	boost::filesystem::file_status const &stat) {

			// FIXME: error handling?
			// FIXME: caching
			// FIXME: modified date
			// FIXME: byte range

			// file-type check:
			if (stat.type() != boost::filesystem::regular_file &&
			    stat.type() != boost::filesystem::symlink_file) {
				rs.status = status_code::not_found;
				return;
			}

			// content-type:
			auto mimep = mime::default_map.find(path.extension().string());
			if (mimep != mime::default_map.end()) {
#ifdef CLANE_HAVE_STD_MULTIMAP_EMPLACE
				rs.headers.emplace("content-type", mimep->second);
#else
				rs.headers.insert(header("content-type", mimep->second));
#endif
			}

			// content-length:
			std::ostringstream ss;
			ss << boost::filesystem::file_size(path);
#ifdef CLANE_HAVE_STD_MULTIMAP_EMPLACE
			rs.headers.emplace("content-length", ss.str());
#else
			rs.headers.insert(header("content-length", ss.str()));
#endif

			// content:
			boost::filesystem::ifstream in(path);
			rs << in.rdbuf();
		}

		void serve_file(response_ostream &rs, request &req, boost::filesystem::path const &path) {
			serve_file(rs, req, path, boost::filesystem::status(path));
		}

		void file_server::operator()(response_ostream &rs, request &req) {
			boost::filesystem::directory_entry ent(root_path / req.uri.path);
			auto stat = ent.status();
			if (stat.type() == boost::filesystem::directory_file)
				serve_dir(rs, req, root_path / req.uri.path);
			else
				serve_file(rs, req, root_path / req.uri.path, stat);
		}

	}
}

