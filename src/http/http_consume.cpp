// vim: set noet:

#include "http_consume.hpp"
#include <algorithm>
#include <cctype>
#include <cstring>
#include <functional>
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
		} token_char_chk;

		// Returns whether a given string comprises valid token characters. This is
		// merely a syntactic check; it does not check whether the method is
		// meaningful.
		static bool is_token(std::string const &s);

		bool is_token(std::string const &s) {
			if (s.empty())
				return false; // token must have at least one character
			for (char c: s) {
				if (!token_char_chk(c))
					return false;
			}
			return true;
		}

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

		void rtrim(std::string &s) {
			static auto is_not_space = std::not1(std::function<int(int)>(static_cast<int(*)(int)>(std::isspace)));
			s.erase(std::find_if(s.rbegin(), s.rend(), is_not_space).base(), s.end());
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

		bool parse_version(int *major_ver, int *minor_ver, std::string &s) {
			if (s.size() < 5 || memcmp(s.c_str(), "HTTP/", 5))
				return false;
			s.erase(0, 5);
			std::istringstream pss(s);
			pss.unsetf(std::ios_base::skipws);
			pss >> *major_ver;
			if (!pss || pss.get() != '.' || *major_ver < 0)
				return false;
			pss >> *minor_ver;
			if (!pss || pss.get() != std::char_traits<char>::eof() || *minor_ver < 0)
				return false;
			return true;
		}

		bool parse_status_code(status_code *stat, std::string &s) {
			std::istringstream pss(s);
			pss.unsetf(pss.skipws);
			int pstat;
			pss >> pstat;
			if (!pss || pss.get() != std::char_traits<char>::eof())
				return false;
			if (!status_code_from_int(*stat, pstat))
				return false;
			return true;
		}

		static char const *skip_whitespace(char const *beg, char const *end);

		char const *skip_whitespace(char const *beg, char const *end) {
			char const *p = beg;
			while (p < end && std::isspace(*p)) {
				++p;
			}
			return p;
		}

		static char const *const error_msg_too_long = "message too long";

		size_t v1x_request_line_consumer::consume(char const *buf, size_t size) {
			static char const *const error_invalid_version = "invalid HTTP version";

			char const *cur = buf;
			char const *const end = buf + size;
			char const *newline = find_newline(cur, end - cur);

			switch (cur_phase) {
				case phase::method: {
					char const *space = reinterpret_cast<char const *>(memchr(cur, ' ', newline - cur));
					if (newline != end && !space) {
						set_error(status_code::bad_request, "missing request line URI reference");
						return error;
					}
					size_t method_len = (space ? space : end) - cur;
					if (!increase_length(method_len + (space ? 1 : 0))) {
						set_error(status_code::bad_request, error_msg_too_long);
						return error;
					}
					method->append(cur, method_len);
					if (!space)
						return end - buf; // incomplete
					if (!is_method_valid(*method)) {
						set_error(status_code::bad_request, "invalid request method");
						return error;
					}
					cur_phase = phase::uri;
					cur = space + 1;
					// fall through switch
				}

	 			case phase::uri: {
					char const *space = reinterpret_cast<char const *>(memchr(cur, ' ', newline - cur));
					if (newline != end && !space) {
						set_error(status_code::bad_request, "missing request line HTTP version");
						return error;
					}
					size_t uri_len = (space ? space : end) - cur;
					if (!increase_length(uri_len + (space ? 1 : 0))) {
						set_error(status_code::request_uri_too_long, "");
						return error;
					}
					uri_str.append(cur, uri_len);
					if (!space)
						return end - buf; // incomplete
					if (!uri::parse_uri_reference(*uri, uri_str)) {
						set_error(status_code::bad_request, "invalid request line URI reference");
						return error;
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
						return error;
					}
					version_str.append(cur, newline - cur);
					if (newline == end)
						return end - buf; // incomplete
					if (!parse_version(major_ver, minor_ver, version_str)) {
						set_error(status_code::bad_request, error_invalid_version);
						return error;
					}
					cur = newline;
					// Invariant: There's at least a carriage return or newline
					// terminating this line.
					if (*cur == '\r') {
						if (!increase_length(1)) {
							set_error(status_code::request_uri_too_long, "");
							return error;
						}
						++cur;
					}
					cur_phase = phase::newline;
					// fall through switch
				}

				case phase::newline: {
					// There's still a newline character needing to be extracted.
					if (cur == end)
						return end - buf; // incomplete
					if (*cur != '\n') {
						set_error(status_code::bad_request, error_invalid_version);
						return error;
					}
					if (!increase_length(1)) {
						set_error(status_code::request_uri_too_long, "");
						return error;
					}
					++cur;
				}
			}
			set_done();
			return cur - buf; // complete and successful
		}

		size_t v1x_status_line_consumer::consume(char const *buf, size_t size) {
			static char const *const error_reason_too_long = "reason phrase too long";

			char const *cur = buf;
			char const *const end = buf + size;
			char const *newline = find_newline(cur, end - cur);

			switch (cur_phase) {

				case phase::version: {
					char const *space = reinterpret_cast<char const *>(memchr(cur, ' ', newline - cur));
					if (newline != end && !space) {
						set_error("missing status code");
						return error;
					}
					size_t version_len = (space ? space : end) - cur;
					if (!increase_length(version_len + (space ? 1 : 0))) {
						set_error("HTTP version too long");
						return error;
					}
					version_str.append(cur, version_len);
					if (!space)
						return end - buf; // incomplete
					if (!parse_version(major_ver, minor_ver, version_str)) {
						set_error("invalid HTTP version");
						return error;
					}
					cur_phase = phase::status;
					cur = space + 1;
					// fall through switch
				}

				case phase::status: {
					char const *space = reinterpret_cast<char const *>(memchr(cur, ' ', newline - cur));
					if (newline != end && !space) {
						set_error("missing reason phrase");
						return error;
					}
					size_t status_len = (space ? space : end) - cur;
					if (!increase_length(status_len + (space ? 1 : 0))) {
						set_error("status code too long");
						return error;
					}
					status_str.append(cur, status_len);
					if (!space)
						return end - buf; // incomplete
					if (!parse_status_code(stat, status_str)) {
						set_error("invalid status code");
						return error;
					}
					cur_phase = phase::reason;
					cur = space + 1;
					// fall through switch
				}

				case phase::reason: {
					if (!increase_length(newline - cur)) {
						set_error(error_reason_too_long);
						return error;
					}
					reason->append(cur, newline - cur);
					if (newline == end)
						return end - buf; // incomplete
					cur = newline;
					// Invariant: There's at least a carriage return or newline
					// terminating this line.
					if (*cur == '\r') {
						if (!increase_length(1)) {
							set_error(error_reason_too_long);
							return error;
						}
						++cur;
					}
					cur_phase = phase::newline;
					// fall through switch
				}

				case phase::newline: {
					// There's still a newline character needing to be extracted.
					if (cur == end)
						return end - buf; // incomplete
					if (*cur != '\n') {
						set_error("invalid reason phrase");
						return error;
					}
					if (!increase_length(1)) {
						set_error(error_reason_too_long);
						return error;
					}
					++cur;
				}
			}
			set_done();
			return cur - buf; // complete and successful
		}

		size_t v1x_headers_consumer::consume(char const *buf, size_t size) {
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
				// TODO: Replace 'insert' with 'emplace' when compilers support it.
				hdrs->insert(header_map::value_type(std::move(hdr_name), std::move(hdr_val)));
				return true;
			};

			char const *cur = buf;
			char const *const end = buf + size;
			char const *newline = find_newline(cur, size);
			if (!increase_length(newline - cur)) {
				set_error(status_code::bad_request, error_msg_too_long);
				return error;
			}

			while (cur < end) {
				switch (cur_phase) {

					case phase::start_line: {
						if (*cur == '\r') {
							cur_phase = phase::end_newline;
							if (!increase_length(1)) {
								set_error(status_code::bad_request, error_msg_too_long);
								return error;
							}
							++cur; // consume carriage return
						} else if (*cur == '\n') {
							if (!increase_length(1)) {
								set_error(status_code::bad_request, error_msg_too_long);
								return error;
							}
							++cur; // consume newline
							if (store_header())
								set_done();
							return cur - buf;
						} else if (*cur == ' ' || *cur == '\t') {
							// linear white space: this line is a continuation of the
							// header value in the previous line
							hdr_val.push_back(' '); // all linear whitespace is replaced by a single space
							cur_phase = phase::pre_value;
						} else {
							if (!store_header())
								return error;
							cur_phase = phase::name;
						}
						break;
					}

					case phase::end_newline: {
						if (*cur != '\n') {
							set_error(status_code::bad_request, error_invalid);
							return error;
						}
						if (!increase_length(1)) {
							set_error(status_code::bad_request, error_msg_too_long);
							return error;
						}
						++cur; // consume newline
						if (!store_header())
							return error;
						set_done();
						return cur - buf;
					}

					case phase::name: {
						char const *colon = reinterpret_cast<char const *>(memchr(cur, ':', end - cur));
						if (!colon)
							colon = end;
						hdr_name.append(cur, colon);
						if (colon == end) {
							if (newline != end) {
								set_error(status_code::bad_request, error_invalid);
								return error;
							}
							return end - buf; // incomplete
						}
						rtrim(hdr_name);
						if (!is_header_name_valid(hdr_name)) {
							set_error(status_code::bad_request, error_invalid);
							return error;
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
							return cur - buf; // incomplete
						if (*cur == '\r') {
							if (!increase_length(1)) {
								set_error(status_code::bad_request, error_msg_too_long);
								return error;
							}
							++cur; // consume carriage return
						}
						cur_phase = phase::value_newline;
						break;
					}

					case phase::value_newline: {
						if (*cur != '\n') {
							set_error(status_code::bad_request, error_invalid);
							return error;
						}
						if (!increase_length(1)) {
							set_error(status_code::bad_request, error_msg_too_long);
							return error;
						}
						++cur; // consume newline
						newline = find_newline(cur, end - cur);
						if (!increase_length(newline - cur)) {
							set_error(status_code::bad_request, error_msg_too_long);
							return error;
						}
						cur_phase = phase::start_line;
						break;
					}
				}
			}

			return cur - buf; // incomplete
		}

		size_t v1x_chunk_line_consumer::consume(char const *buf, size_t size) {
			static const char *invalid = "invalid chunk size";
			size_t i = 0;
			while (true) {
				if (i == size)
					return size; // incomplete
				switch (cur_phase) {
					case phase::digit:
						if (nibs == max_nibs) {
							set_error(status_code::bad_request, "chunk size too big");
							return error;
						}
						if (('\r' == buf[i] || '\n' == buf[i]) && !nibs) {
							set_error(status_code::bad_request, invalid);
							return error;
						}
						if ('\r' == buf[i]) {
							increase_length(1);
							cur_phase = phase::newline;
							break;
						}
						if ('\n' == buf[i]) {
							increase_length(1);
							++i;
							return i;
						}
						if (!isxdigit(buf[i])) {
							set_error(status_code::bad_request, invalid);
							return error;
						}
						val <<= 4;
						++nibs;
						increase_length(1);
						if ('0' <= buf[i] && buf[i] <= '9')
							val |= buf[i] - '0';
						else if ('A' <= buf[i] && buf[i] <= 'F')
							val |= buf[i] - 'A' + 10;
						else
							val |= buf[i] - 'a' + 10;
						break;
					case phase::newline:
						if ('\n' != buf[i]) {
							set_error(status_code::bad_request, invalid);
							return error;
						}
						if (!nibs) {
							set_error(status_code::bad_request, invalid);
							return error;
						}
						increase_length(1);
						++i;
						return i;
				}
				++i;
			}
		}

#if 0
		bool request_1x_consumer::consume(char const *buf, size_t size) {
			pre_consume();
			char const *cur = buf;
			char const *const end = buf + size;
			while (true) {
				switch (cur_phase) {
					case phase::request_line: {
						size_t len = total_length();
						if (!request_line_consumer::consume(cur, end - cur))
							return false;
						if (!request_line_consumer::operator bool())
							return true;
						cur += total_length() - len;
						cur_phase = phase::headers;
						break;
					}
					case phase::headers: {
						size_t len = total_length();
						if (!headers_consumer::consume(cur, end - cur))
							return false;
						if (!headers_consumer::operator bool())
							return true;
						cur += total_length() - len;
						return true; // success
					}
				}
			}
		}

#endif
	}
}

