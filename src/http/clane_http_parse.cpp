// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// vim: set noet:

/** @file */

#include "clane_http_parse.hpp"
#include "../ascii/clane_ascii.hpp"
#include <cstring>
#include <sstream>

namespace clane {
	namespace http {

		// Functor that determines whether a given character is a valid token
		// character. The valid token characters are specified in RFC 2616
		// section 2.2 ("Basic Rules").
		static class token_char_checker {
			bool ok_chars[128];
		public:
			token_char_checker() {

				// all CHAR minus CTL
				for (int i = 0; i < 32; ++i)
					ok_chars[i] = false;
				for (int i = 32; i < 127; ++i)
					ok_chars[i] = true;
				ok_chars[127] = false;

				// minus other characters:
				ok_chars[static_cast<int>('(')] = false;
				ok_chars[static_cast<int>(')')] = false;
				ok_chars[static_cast<int>('<')] = false;
				ok_chars[static_cast<int>('>')] = false;
				ok_chars[static_cast<int>('@')] = false;
				ok_chars[static_cast<int>(',')] = false;
				ok_chars[static_cast<int>(';')] = false;
				ok_chars[static_cast<int>(':')] = false;
				ok_chars[static_cast<int>('\\')] = false;
				ok_chars[static_cast<int>('\"')] = false;
				ok_chars[static_cast<int>('/')] = false;
				ok_chars[static_cast<int>('[')] = false;
				ok_chars[static_cast<int>(']')] = false;
				ok_chars[static_cast<int>('?')] = false;
				ok_chars[static_cast<int>('=')] = false;
				ok_chars[static_cast<int>('{')] = false;
				ok_chars[static_cast<int>('}')] = false;
				ok_chars[static_cast<int>(' ')] = false;
				ok_chars[static_cast<int>('\t')] = false;
			}

			bool operator()(char c) const {
				return 0 <= c && c < 128 && ok_chars[static_cast<int>(c)];
			}
		} is_token_char;

		// Returns whether a given string comprises valid token characters. This is
		// merely a syntactic check; it does not check whether the token is
		// meaningful.
		static bool is_token(std::string const &s) {
			if (s.empty())
				return false; // token must have at least one character
			return std::find_if_not(s.begin(), s.end(), is_token_char) == s.end();
		}

		static bool status_code_from_int(status_code &stat, int n) {
			if (!*what(static_cast<status_code>(n)))
				return false;
			stat = static_cast<status_code>(n);
			return true;
		}

		bool is_header_name_valid(std::string const &s) {
			return is_token(s);
		}

		bool is_header_value_valid(std::string const &s) {
			for (size_t i = 0; i < s.size(); ++i) {
				if ((0 <= s[i] && s[i] < 32 && s[i] != '\t') || s[i] == 127)
					return false;
			}
			return true;
		}

		bool is_method_valid(std::string const &s) {
			return is_token(s);
		}

		bool parse_version(int *major_ver, int *minor_ver, std::string &s) {
			if (s.size() < 5 || std::memcmp(s.c_str(), "HTTP/", 5))
				return false;
			s.erase(0, 5);
			std::istringstream pss(s);
			pss.unsetf(pss.skipws);
			pss >> *major_ver;
			if (!pss || pss.get() != '.' || *major_ver < 0)
				return false;
			pss >> *minor_ver;
			if (!pss || pss.get() != std::istringstream::traits_type::eof() || *minor_ver < 0)
				return false;
			return true;
		}

		bool parse_status_code(status_code *stat, std::string &s) {
			std::istringstream pss(s);
			pss.unsetf(pss.skipws);
			int pstat;
			pss >> pstat;
			if (!pss || pss.get() != std::istringstream::traits_type::eof())
				return false;
			if (!status_code_from_int(*stat, pstat))
				return false;
			return true;
		}

		bool query_headers_chunked(header_map const &hdrs) {
			// FIXME: Check for other transfer-codings.
			// FIXME: Check transfer-encoding order.
			static std::string const key("transfer-encoding");
			auto r = hdrs.equal_range(key);
			for (auto i = r.first; i != r.second; ++i) {
				if (i->second == "chunked")
					return true;
			}
			return false;
		}

		bool query_headers_content_length(header_map const &hdrs, size_t &olen) {
			// FIXME: Check for multiple and/or invalid content-length headers. For
			// now, assume the first content-length header is the only one.
			static std::string const key("content-length");
			auto p = hdrs.find(key);
			if (p == hdrs.end())
				return false;
			std::istringstream ss(p->second);
			size_t len;
			ss >> len;
			if (!ss || !ss.eof())
				return false;
			olen = len;
			return true;
		}

		void v1x_request_line_incparser::reset() {
			incparser::reset();
			cur_stat = state::method;
			method_.clear();
			uri_.clear();
			uri_str.clear();
			version_str.clear();
		}

		size_t v1x_request_line_incparser::parse_some(char const *beg, char const *end) {
			static char const *const uri_ref_too_long = "request line URI reference too long";
			static char const *const invalid_ver = "invalid request line HTTP version";
			char const *cur = beg;
			char const *newline = ascii::find_newline(cur, end);
			switch (cur_stat) {
				case state::method: {
					char const *space = std::find(cur, newline, ' ');
					if (newline != end && space == newline) {
						set_error(status_code::bad_request, "missing request line URI reference");
						return error;
					}
					size_t method_len = space - cur;
					if (!increase_length(method_len + (space!=newline ? 1 : 0))) {
						set_error(status_code::bad_request, "request line method too long");
						return error;
					}
					method_.append(cur, method_len);
					if (space == newline)
						return end - beg; // incomplete
					if (!is_method_valid(method_)) {
						set_error(status_code::bad_request, "invalid request line method");
						return error;
					}
					cur_stat = state::uri;
					cur = space + 1;
					// fall through to next case
				}

	 			case state::uri: {
					char const *space = std::find(cur, newline, ' ');
					if (newline != end && space == newline) {
						set_error(status_code::bad_request, "missing request line HTTP version");
						return error;
					}
					size_t uri_len = space - cur;
					if (!increase_length(uri_len + (space!=newline ? 1 : 0))) {
						set_error(status_code::request_uri_too_long, uri_ref_too_long);
						return error;
					}
					uri_str.append(cur, uri_len);
					if (space == newline)
						return newline - beg; // incomplete
					std::error_code e;
					uri_ = uri::parse_uri_reference(uri_str.data(), uri_str.data()+uri_str.size(), e);
					if (e) {
						set_error(status_code::bad_request, "invalid request line URI reference");
						return error;
					}
					cur_stat = state::version;
					cur = space + 1;
					// fall through to next case
				}

				case state::version: {
					if (!increase_length(newline - cur)) {
						// Assume the URI reference was too long, which has caused the
						// version string to exceed the length limit. If the problem is
						// really that the version string is too long then the client will
						// receive a misleading error code, which is acceptable because the
						// client did something nonstandard and bizarre.
						set_error(status_code::request_uri_too_long, uri_ref_too_long);
						return error;
					}
					version_str.append(cur, newline - cur);
					if (newline == end)
						return newline - beg; // incomplete
					if (!parse_version(&major_ver, &minor_ver, version_str)) {
						set_error(status_code::bad_request, invalid_ver);
						return error;
					}
					cur = newline;
					// Invariant: There's at least a carriage return or newline
					// terminating this line.
					if (*cur == '\r') {
						if (!increase_length(1)) {
							set_error(status_code::request_uri_too_long, uri_ref_too_long);
							return error;
						}
						++cur;
					}
					cur_stat = state::newline;
					// fall through to next case
				}

				case state::newline: {
					// There's still a newline character needing to be extracted.
					if (cur == end)
						return end - beg; // incomplete
					if (*cur != '\n') {
						set_error(status_code::bad_request, invalid_ver);
						return error;
					}
					if (!increase_length(1)) {
						set_error(status_code::request_uri_too_long, uri_ref_too_long);
						return error;
					}
					++cur;
				}
			}
			set_done();
			return cur - beg; // complete and successful
		}

		void v1x_status_line_incparser::reset() {
			incparser::reset();
			cur_stat = state::version;
			reason_.clear();
			version_str.clear();
			status_str.clear();
		}

		size_t v1x_status_line_incparser::parse_some(char const *beg, char const *end) {
			static char const *const too_long = "status line too long";
			char const *cur = beg;
			char const *newline = ascii::find_newline(cur, end);
			switch (cur_stat) {

				case state::version: {
					char const *space = std::find(cur, newline, ' ');
					if (newline != end && space == newline) {
						set_error(status_code::bad_request, "missing status line status code");
						return error;
					}
					size_t version_len = space - cur;
					if (!increase_length(version_len + (space!=newline ? 1 : 0))) {
						set_error(status_code::bad_request, too_long);
						return error;
					}
					version_str.append(cur, version_len);
					if (space == newline)
						return newline - beg; // incomplete
					if (!parse_version(&major_ver, &minor_ver, version_str)) {
						set_error(status_code::bad_request, "invalid status line HTTP version");
						return error;
					}
					cur_stat = state::status;
					cur = space + 1;
					// fall through to next case
				}

				case state::status: {
					char const *space = std::find(cur, newline, ' ');
					if (newline != end && space == newline) {
						set_error(status_code::bad_request, "missing status line reason phrase");
						return error;
					}
					size_t status_len = space - cur;
					if (!increase_length(status_len + (space!=newline ? 1 : 0))) {
						set_error(status_code::bad_request, too_long);
						return error;
					}
					status_str.append(cur, status_len);
					if (space == newline)
						return newline - beg; // incomplete
					if (!parse_status_code(&status_, status_str)) {
						set_error(status_code::bad_request, "invalid status line status code");
						return error;
					}
					cur_stat = state::reason;
					cur = space + 1;
					// fall through to next case
				}

				case state::reason: {
					if (!increase_length(newline - cur)) {
						set_error(status_code::bad_request, too_long);
						return error;
					}
					reason_.append(cur, newline - cur);
					if (newline == end)
						return end - beg; // incomplete
					cur = newline;
					// Invariant: There's at least a carriage return or newline
					// terminating this line.
					if (*cur == '\r') {
						if (!increase_length(1)) {
							set_error(status_code::bad_request, too_long);
							return error;
						}
						++cur;
					}
					cur_stat = state::newline;
					// fall through to next case
				}

				case state::newline: {
					// There's still a newline character needing to be extracted.
					if (cur == end)
						return end - beg; // incomplete
					if (*cur != '\n') {
						set_error(status_code::bad_request, "invalid status line reason phrase");
						return error;
					}
					if (!increase_length(1)) {
						set_error(status_code::bad_request, too_long);
						return error;
					}
					++cur;
				}
			}
			set_done();
			return cur - beg; // complete and successful
		}

		void v1x_headers_incparser::reset() {
			incparser::reset();
			cur_stat = state::start_line;
			hdrs.clear();
			hdr_name.clear();
			hdr_val.clear();
		}

		size_t v1x_headers_incparser::parse_some(char const *beg, char const *end) {

			static char const *const too_long = "HTTP headers too long";
			static char const *const invalid = "invalid HTTP header";
			char const *cur = beg;
			char const *newline = ascii::find_newline(cur, end);

			// Invariant: The newline variable points to the end of the line, or the
			// end of the input.

			if (!increase_length(newline - cur)) {
				set_error(status_code::bad_request, too_long);
				return error;
			}

			// Invariant: The parser's current length includes the current line's
			// length.

			while (cur < end) {
				switch (cur_stat) {

					case state::start_line: {
						if (*cur == '\r') {
							cur_stat = state::end_newline;
							if (!increase_length(1)) {
								set_error(status_code::bad_request, too_long);
								return error;
							}
							++cur; // consume carriage return
						} else if (*cur == '\n') {
							if (!increase_length(1)) {
								set_error(status_code::bad_request, too_long);
								return error;
							}
							++cur; // consume newline
							set_done();
							return cur - beg;
						} else {
							cur_stat = state::name;
						}
						break;
					}

					case state::end_newline: {
						if (*cur != '\n') {
							set_error(status_code::bad_request, invalid);
							return error;
						}
						if (!increase_length(1)) {
							set_error(status_code::bad_request, too_long);
							return error;
						}
						++cur; // consume newline
						set_done();
						return cur - beg;
					}

					case state::name: {
						char const *colon = std::find(cur, newline, ':');
						hdr_name.append(cur, colon);
						if (colon == newline) {
							if (newline != end) {
								set_error(status_code::bad_request, invalid);
								return error;
							}
							cur = colon;
							return cur - beg; // incomplete
						}
						ascii::rtrim(hdr_name);
						if (!is_header_name_valid(hdr_name)) {
							set_error(status_code::bad_request, invalid);
							return error;
						}
						cur_stat = state::value_skipws;
						cur = colon + 1;
						break;
					}

					case state::value_skipws: {
						cur = ascii::skip_whitespace(cur, newline);
						if (cur != newline)
							cur_stat = state::value;
						break;
					}

					case state::value: {
						hdr_val.append(cur, newline);
						cur = newline;
						if (cur == end)
							return cur - beg; // incomplete
						if (*cur == '\r') {
							if (!increase_length(1)) {
								set_error(status_code::bad_request, too_long);
								return error;
							}
							++cur; // consume carriage return
						}
						cur_stat = state::value_newline;
						break;
					}

					case state::value_newline: {
						if (*cur != '\n') {
							set_error(status_code::bad_request, invalid);
							return error;
						}
						if (!increase_length(1)) {
							set_error(status_code::bad_request, too_long);
							return error;
						}
						++cur; // consume newline
						newline = ascii::find_newline(cur, end);
						if (!increase_length(newline - cur)) {
							set_error(status_code::bad_request, too_long);
							return error;
						}
						cur_stat = state::value_start_line;
						break;
					}

					case state::value_start_line: {
						char const *nonspace = cur;
						while (nonspace < newline && (*nonspace == ' ' || *nonspace == '\t'))
							++nonspace;
						if (cur != nonspace) {
							cur = nonspace;
							hdr_val.push_back(' '); // replace all linear whitespace with a single space character
							cur_stat = state::value_skipws;
							break;
						}
						ascii::rtrim(hdr_val);
						if (!is_header_value_valid(hdr_val)) {
							set_error(status_code::bad_request, invalid);
							return error;
						}
#ifdef CLANE_HAVE_STD_MULTIMAP_EMPLACE
						hdrs.emplace(std::move(hdr_name), std::move(hdr_val));
#else
						hdrs.insert(header(std::move(hdr_name), std::move(hdr_val)));
#endif
						hdr_name.clear();
						hdr_val.clear();
						cur_stat = state::start_line;
					}
				}
			}

			return cur - beg; // incomplete
		}

		void v1x_chunk_line_incparser::reset() {
			incparser::reset();
			cur_stat = state::digit;
			nibs = 0;
			chunk_size_ = 0;
		}

		size_t v1x_chunk_line_incparser::parse_some(char const *beg, char const *end) {
			static char const *const invalid = "invalid chunk size digit";
			char const *cur = beg;
			while (cur < end) {
				switch (cur_stat) {
					case state::digit:
						if (nibs == max_nibs) {
							set_error(status_code::bad_request, "chunk size overflow");
							return error;
						}
						if ('\r' == *cur || '\n' == *cur) {
							if (!nibs) {
								set_error(status_code::bad_request, "missing chunk size");
								return error;
							}
							if ('\r' == *cur) {
								cur_stat = state::newline;
								break;
							}
							++cur; // consume newline
							set_done();
							return cur - beg;
						}
						if (!std::isxdigit(*cur)) {
							set_error(status_code::bad_request, invalid);
							return error;
						}
						chunk_size_ <<= 4;
						chunk_size_ |= ascii::hexch_to_int(*cur);
						++nibs;
						break;
					case state::newline:
						if ('\n' != *cur) {
							set_error(status_code::bad_request, invalid);
							return error;
						}
						++cur; // consume newline
						set_done();
						return cur - beg;
				}
				++cur; // consume hex digit or carriage return
			}
			return cur - beg; // incomplete
		}

		void v1x_body_incparser::reset(length_type len_type, size_t len) {
			incparser::reset();
			this->len_type = len_type;
			switch (len_type) {
				case fixed:
					cur_stat = state::body_data;
					rem = len;
					break;
				case chunked:
					cur_stat = state::chunk_line;
					chunk_pars.reset();
					size_ = 0;
					break;
				case infinite:
					cur_stat = state::body_data;
					rem = 0;
					break;
			}
		}

		size_t v1x_body_incparser::parse_some(char const *beg, char const *end) {
			static char const *const invalid = "chunk incorrectly terminated";
			char const *cur = beg;
			bool repeat = true;
			while (repeat) {
				repeat = false;
				switch (cur_stat) {

					case state::chunk_carriage_return:
						size_ = 0;
						if (cur == end)
							return cur - beg; // incomplete
						if (*cur == '\n') {
							++cur; // consume newline
							cur_stat = state::chunk_line;
							chunk_pars.reset();
							repeat = true;
							break;
						}
						if (*cur != '\r') {
							set_error(status_code::bad_request, invalid);
							return error;
						}
						++cur; // consume carriage return
						cur_stat = state::chunk_newline;
						// fall through to next case

					case state::chunk_newline:
						if (cur == end)
							return cur - beg; // incomplete
						if (*cur != '\n') {
							set_error(status_code::bad_request, invalid);
							return error;
						}
						++cur; // consume newline
						cur_stat = state::chunk_line;
						chunk_pars.reset();
						// fall through to next case

					case state::chunk_line: {
						// Invariant: size_ == 0.
						size_t stat = chunk_pars.parse_some(cur, end);
						if (error == stat) {
							set_error(chunk_pars.status(), chunk_pars.what());
							return error;
						}
						cur += stat;
						if (chunk_pars)
							return cur - beg; // incomplete, no body data available
						rem = chunk_pars.chunk_size();
						if (!rem) {
							set_done(); // terminator chunk line, no body data available
							return cur - beg;
						}
						cur_stat = state::body_data;
						// fall through to next case
					}

					case state::body_data: {
						// Pass control to caller to process the body data.
						offset_ = cur - beg;
						size_t n = end - cur;
						size_t got = std::min(rem ? rem : n, n);
						cur += got;
						size_ = got;
						if (rem)
							rem -= got;
						if (len_type == fixed && !rem)
							set_done(); // received all data
						else if (len_type == chunked && !rem)
							cur_stat = state::chunk_carriage_return;
					}
				}
			}

			return cur - beg; // maybe complete, maybe not
		}

		void v1x_request_incparser::reset() {
			cur_stat = state::request_line;
			v1x_request_line_incparser::reset();
			got_hdrs = false;
		}

		size_t v1x_request_incparser::parse_some(char const *beg, char const *end) {
			char const *cur = beg;
			switch (cur_stat) {
				case state::request_line: {
					size_t stat = v1x_request_line_incparser::parse_some(cur, end);
					if (error == stat)
						return error;
					cur += stat;
					if (*this)
						return cur - beg; // incomplete
					cur_stat = state::headers;
					v1x_headers_incparser::reset();
					// fall through to next case
				}

				case state::headers: {
					size_t stat = v1x_headers_incparser::parse_some(cur, end);
					if (error == stat)
						return error;
					cur += stat;
					if (*this)
						return cur - beg; // incomplete
					got_hdrs = true;
					hdrs = std::move(v1x_headers_incparser::headers());
					cur_stat = state::body;
					offset_ = size_ = 0;
					size_t len;
					if ((chunked = query_headers_chunked(hdrs)))
						v1x_body_incparser::reset(v1x_body_incparser::chunked, 0);
					else if (query_headers_content_length(hdrs, len)) {
						if (!len) {
							v1x_headers_incparser::headers().clear(); // no trailers
							return cur - beg; // already set as done
						}
						v1x_body_incparser::reset(v1x_body_incparser::fixed, len);
					} else {
						v1x_headers_incparser::headers().clear(); // no trailers
						return cur - beg; // already set as done
					}
					return cur - beg; // mandatory return before parsing any of the body
				}

				case state::body: {
					size_t stat = v1x_body_incparser::parse_some(cur, end);
					if (error == stat)
						return error;
					offset_ = (cur - beg) + v1x_body_incparser::offset();
					size_ = v1x_body_incparser::size();
					cur += stat;
					if (!*this && chunked) {
						cur_stat = state::pre_trailers;
						v1x_headers_incparser::reset();
					}
					return cur - beg; // maybe done, maybe incomplete
				}

				case state::pre_trailers: {
					size_ = 0;
					cur_stat = state::trailers;
					// fall through to next case
				}

				case state::trailers: {
					size_t stat = v1x_headers_incparser::parse_some(cur, end);
					if (error == stat)
						return error;
					cur += stat;
					break;
				}
			}
			return cur - beg; // maybe done, maybe not
		}

		void v1x_response_incparser::reset() {
			cur_stat = state::status_line;
			v1x_status_line_incparser::reset();
			got_hdrs = false;
		}

		size_t v1x_response_incparser::parse_some(char const *beg, char const *end) {
			char const *cur = beg;
			switch (cur_stat) {
				case state::status_line: {
					size_t stat = v1x_status_line_incparser::parse_some(cur, end);
					if (error == stat)
						return error;
					cur += stat;
					if (*this)
						return cur - beg; // incomplete
					cur_stat = state::headers;
					v1x_headers_incparser::reset();
					// fall through to next case
				}

				case state::headers: {
					size_t stat = v1x_headers_incparser::parse_some(cur, end);
					if (error == stat)
						return error;
					cur += stat;
					if (*this)
						return cur - beg; // incomplete
					got_hdrs = true;
					hdrs = std::move(v1x_headers_incparser::headers());
					cur_stat = state::body;
					offset_ = size_ = 0;
					size_t len;
					if ((chunked = query_headers_chunked(hdrs)))
						v1x_body_incparser::reset(v1x_body_incparser::chunked, 0);
					else if (query_headers_content_length(hdrs, len)) {
						if (!len) {
							v1x_headers_incparser::headers().clear(); // no trailers
							return cur - beg; // already set as done
						}
						v1x_body_incparser::reset(v1x_body_incparser::fixed, len);
					} else {
						v1x_headers_incparser::headers().clear(); // no trailers
						v1x_body_incparser::reset(v1x_body_incparser::infinite, 0); // body ends with connection
					}
					return cur - beg; // mandatory return before parsing any of the body
				}

				case state::body: {
					size_t stat = v1x_body_incparser::parse_some(cur, end);
					if (error == stat)
						return error;
					offset_ = (cur - beg) + v1x_body_incparser::offset();
					size_ = v1x_body_incparser::size();
					cur += stat;
					if (!*this && chunked) {
						cur_stat = state::pre_trailers;
						v1x_headers_incparser::reset();
					}
					return cur - beg; // maybe done, maybe incomplete
				}

				case state::pre_trailers: {
					size_ = 0;
					cur_stat = state::trailers;
					// fall through to next case
				}

				case state::trailers: {
					size_t stat = v1x_headers_incparser::parse_some(cur, end);
					if (error == stat)
						return error;
					cur += stat;
					break;
				}
			}
			return cur - beg; // maybe done, maybe not
		}

	}
}

