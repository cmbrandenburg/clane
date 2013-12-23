// vim: set noet:

/** @file */

#include "http_parse.h"
#include <algorithm>
#include <cctype>
#include <cstring>
#include <functional>
#include <sstream>

namespace clane {
	namespace http {

		static char const *const error_too_long = "message too long";

		// Functor that determines whether a given character is a valid token
		// character. The valid token characters are specified in RFC 2616
		// section 2.2 ("Basic Rules").
		struct token_char_checker {
		private:
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
		};

		static token_char_checker const token_char_chk;

		// Searches a given memory block for the first newline. If a carriage return
		// and newline pair is found ("\r\n") then this returns a pointer to the
		// carriage return character. Else, if a newline is found then this returns
		// a pointer to the newline character. Else, if a carriage return is found
		// at the last byte of the block then this returns a pointer to that
		// carriage return. Else, this returns a pointer to the first byte after the
		// memory block.
		//
		// In other words, the result of this function is to return a pointer to the
		// first "unreadable" byte in or out of the block, where readable characters
		// are characters in the current line, excluding "\r\n" and "\n". Note that
		// carriage returns followed by a character other than a newline are
		// considered readable.
		static char const *find_newline(const char *p, size_t n);

		static bool is_header_name_valid(std::string const &s);
		static bool is_header_value_valid(std::string const &s);
		static bool is_method_valid(std::string const &s);

		static char const *skip_whitespace(char const *beg, char const *end);
		static void rtrim(std::string &s);

		// Returns whether a given string comprises valid token characters. This is
		// merely a syntactic check; it does not check whether the method is
		// meaningful.
		static bool is_token(std::string const &s);

		char const *find_newline(char const *p, size_t n) {
			char const *newline = reinterpret_cast<char const *>(memchr(p, '\n', n));
			if (newline && newline > p && *(newline - 1) == '\r')
				return newline - 1; // carriage return before newline
			if (newline)
				return newline; // newline
			if (n > 0 && *(p + n - 1) == '\r')
				return p + n - 1; // carriage return at end of block
			return p + n; // no carriage return or newline
		}

		bool is_header_name_valid(std::string const &s) {
			return is_token(s);
		}

		bool is_header_value_valid(std::string const &s) {
			for (char i: s) {
				if (0 <= i && i < 32 && i != '\t')
					return false;
				if (i == 127)
					return false;
			}
			return true;
		}

		bool is_method_valid(std::string const &s) {
			return is_token(s);
		}

		bool is_token(std::string const &s) {
			if (s.empty())
				return false; // token must have at least one character
			for (char c: s) {
				if (!token_char_chk(c))
					return false;
			}
			return true;
		}

		void rtrim(std::string &s) {
			s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
		}

		char const *skip_whitespace(char const *beg, char const *end) {
			char const *p = beg;
			while (p < end && std::isspace(*p)) {
				++p;
			}
			return p;
		}

		headers_parser::headers_parser(): cur_phase(phase::start_line) {
		}

		bool headers_parser::parse(char const *buf, size_t size) {
			static char const *const error_invalid = "invalid message header";

			auto store_header = [&]() -> bool {
				if (hdr_name.empty())
					return true; // special case: ignore empty line
				// The header name has already been validated.
				rtrim(hdr_val);
				if (!is_header_value_valid(hdr_val)) {
					set_error(status_code::bad_request, error_invalid);
					return false;
				}
				// FIXME: replace 'insert' with 'emplace'?
				hdrs.insert(header_map::value_type(std::move(hdr_name), std::move(hdr_val)));
				return true;
			};

			char const *cur = buf;
			char const *const end = buf + size;
			char const *newline = find_newline(cur, size);
			if (!increase_length(newline - cur)) {
				set_error(status_code::bad_request, error_too_long);
				return false;
			}

			while (cur < end) {
				switch (cur_phase) {

					case phase::start_line: {
						if (*cur == '\r') {
							cur_phase = phase::end_newline;
							if (!increase_length(1)) {
								set_error(status_code::bad_request, error_too_long);
								return false;
							}
							++cur; // consume carriage return
						} else if (*cur == '\n') {
							if (!increase_length(1)) {
								set_error(status_code::bad_request, error_too_long);
								return false;
							}
							++cur; // consume newline
							return store_header();
						} else if (*cur == ' ' || *cur == '\t') {
							// linear white space: this line is a continuation of the
							// header value in the previous line
							hdr_val.push_back(' '); // all linear whitespace is replaced by a single space
							cur_phase = phase::pre_value;
						} else {
							if (!store_header())
								return false;
							cur_phase = phase::name;
						}
						break;
					}

					case phase::end_newline: {
						if (*cur != '\n') {
							set_error(status_code::bad_request, error_invalid);
							return false;
						}
						if (!increase_length(1)) {
							set_error(status_code::bad_request, error_too_long);
							return false;
						}
						++cur; // consume newline
						return store_header();
					}

					case phase::name: {
						char const *colon = reinterpret_cast<char const *>(memchr(cur, ':', end - cur));
						if (!colon)
							colon = end;
						hdr_name.append(cur, colon);
						if (colon == end)
							return false; // incomplete
						rtrim(hdr_name);
						if (!is_header_name_valid(hdr_name)) {
							set_error(status_code::bad_request, error_invalid);
							return false;
						}
						cur_phase = phase::pre_value;
						cur = colon + 1;
						break;
					}

					case phase::pre_value: {
						cur = skip_whitespace(cur, end);
						if (cur != end)
							cur_phase = phase::value;
						break;
					}

					case phase::value: {
						hdr_val.append(cur, newline);
						cur = newline;
						if (cur == end)
							return false; // incomplete
						if (*cur == '\r') {
							if (!increase_length(1)) {
								set_error(status_code::bad_request, error_too_long);
								return false;
							}
							++cur; // consume carriage return
						}
						cur_phase = phase::value_newline;
						break;
					}

					case phase::value_newline: {
						if (*cur != '\n') {
							set_error(status_code::bad_request, error_invalid);
							return false;
						}
						if (!increase_length(1)) {
							set_error(status_code::bad_request, error_too_long);
							return false;
						}
						++cur; // consume newline
						newline = find_newline(cur, end - cur);
						if (!increase_length(newline - cur)) {
							set_error(status_code::bad_request, error_too_long);
							return false;
						}
						cur_phase = phase::start_line;
						break;
					}
				}
			}

			return false; // incomplete
		}

		void headers_parser::reset() {
			parser::reset();
			cur_phase = phase::start_line;
			hdrs.clear();
			hdr_name.clear();
			hdr_val.clear();
		}

		bool parser::increase_length(size_t n) {
			size_t new_len = cur_len + n;
			if (new_len < cur_len)
				return false; // overflow
			if (len_limit && new_len > len_limit)
				return false; // length limit set and exceeded
			cur_len = new_len;
			return true;
		}

		parser::parser(): stat(status::ready), len_limit{}, cur_len{} {
		}

		void parser::reset() {
			stat = status::ready;
			cur_len = 0;
			// The length limit is preserved.
		}

		void parser::set_error(status_code n, char const *what) {
			stat = status::error;
			error_code_ = n;
			what_ = what;
		}

		bool request_line_parser::parse(char const *buf, size_t size) {
			static char const *const error_invalid_version = "invalid HTTP version";

			char const *cur = buf;
			char const *const end = buf + size;
			char const *newline = find_newline(cur, end - cur);

			switch (cur_phase) {
				case phase::method: {
					char const *space = reinterpret_cast<char const *>(memchr(cur, ' ', newline - cur));
					if (newline != end && !space) {
						set_error(status_code::bad_request, "missing request line URI reference");
						return false;
					}
					size_t method_len = (space ? space : newline) - cur;
					if (!increase_length(method_len + (space ? 1 : 0))) {
						set_error(status_code::bad_request, error_too_long);
						return false;
					}
					method_.append(cur, method_len);
					if (!space)
						return false; // incomplete
					if (!is_method_valid(method_)) {
						set_error(status_code::bad_request, "invalid request method");
						return false;
					}
					cur_phase = phase::uri;
					cur = space + 1;
					// fall through switch
				}

	 			case phase::uri: {
					char const *space = reinterpret_cast<char const *>(memchr(cur, ' ', newline - cur));
					if (newline != end && !space) {
						set_error(status_code::bad_request, "missing request line HTTP version");
						return false;
					}
					size_t uri_len = (space ? space : newline) - cur;
					if (!increase_length(uri_len + (space ? 1 : 0))) {
						set_error(status_code::request_uri_too_long, "");
						return false;
					}
					uri_str.append(cur, uri_len);
					if (!space)
						return false; // incomplete
					if (!uri::parse_uri_reference(uri_, uri_str)) {
						set_error(status_code::bad_request, "invalid request line URI reference");
						return false;
					}
					cur_phase = phase::version;
					cur = space + 1;
					// fall through switch
				}

				case phase::version: {
					if (!increase_length(newline - cur)) {
						// Assume the URI reference was too long, which has caused the
						// version string to exceed the length limit. If the problem is
						// really that the version string is too long then the client will
						// receive a misleading error code, which is acceptable because the
						// client did something excessively non-standard.
						set_error(status_code::request_uri_too_long, "");
						return false;
					}
					version_str.append(cur, newline - cur);
					if (newline == end)
						return false; // incomplete
					if (!parse_version()) {
						set_error(status_code::bad_request, error_invalid_version);
						return false;
					}
					cur = newline;
					// Invariant: There's at least a carriage return or newline
					// terminating this line.
					if (*cur == '\r') {
						if (!increase_length(1)) {
							set_error(status_code::request_uri_too_long, "");
							return false;
						}
						++cur;
					}
					cur_phase = phase::newline;
					// fall through switch
				}

				case phase::newline: {
					// There's still a newline character needing to be extracted.
					if (cur == end)
						return false; // incomplete
					if (*cur != '\n') {
						set_error(status_code::bad_request, error_invalid_version);
						return false;
					}
					if (!increase_length(1)) {
						set_error(status_code::request_uri_too_long, "");
						return false;
					}
				}
			}
			return true; // complete and successful
		}

		void request_line_parser::reset() {
			parser::reset();
			cur_phase = phase::method;
			method_.clear();
			uri_.clear();
			uri_str.clear();
			version_str.clear();
		}

		bool request_line_parser::parse_version() {
			if (version_str.size() < 5 || memcmp(version_str.c_str(), "HTTP/", 5))
				return false;
			version_str.erase(0, 5);
			std::stringstream pss(version_str);
			pss.unsetf(std::ios_base::skipws);
			pss >> major_ver;
			if (!pss || pss.get() != '.')
				return false;
			pss >> minor_ver;
			if (!pss || pss.get() != std::char_traits<char>::eof())
				return false;
			return true;
		}

		request_line_parser::request_line_parser(): cur_phase(phase::method) {
		}

		bool request_1x_parser::parse(char const *buf, size_t size) {
			char const *cur = buf;
			char const *const end = buf + size;
			size_t delta;
			while (true) {
				switch (cur_phase) {
					case phase::request_line:
						if (!req_line_parser.parse(cur, end - cur)) {
							if (!req_line_parser)
								set_error(req_line_parser.error_code(), req_line_parser.what());
							return false;
						}
						delta = req_line_parser.parse_length();
						if (!increase_length(delta)) {
							set_error(status_code::bad_request, error_too_long);
							return false;
						}
						cur += delta;
						cur_phase = phase::headers;
						break;
					case phase::headers:
						if (!hdrs_parser.parse(cur, end - cur)) {
							if (!hdrs_parser)
								set_error(hdrs_parser.error_code(), hdrs_parser.what());
							return false;
						}
						delta = hdrs_parser.parse_length();
						if (!increase_length(delta)) {
							set_error(status_code::bad_request, error_too_long);
							return false;
						}
						cur += delta;
						return true; // success
						break;
				}
			}
		};

		request_1x_parser::request_1x_parser(): cur_phase(phase::request_line) {
		}

		void request_1x_parser::reset() {
			parser::reset();
			cur_phase = phase::request_line;
			req_line_parser.reset();
			hdrs_parser.reset();
		}

	}
}

