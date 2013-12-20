// vim: set noet:

#ifndef CLANE_PARSE_H
#define CLANE_PARSE_H

#include "../http_parse.h"
#include "../../check/check.h"
#include <memory>
#include <vector>

namespace clane {

	struct parser_input {
		std::unique_ptr<char[]> str;
		size_t cap;
		size_t offset;
		size_t size;
	};

	parser_input make_parser_input(char const *prefix, char const *cons_str, char const *rem_str);

	template <class Parser>
	void check_parse_nok_(char const *sname, int sline, Parser &parser, size_t len_limit,
			char const *cons_str, char const *rem_str, std::function<void()> const &checker) {

		std::cerr << "trying " << sname << ":" << sline << std::endl;
		parser.len_limit = len_limit;

		// full string at once:
		{
			clane::parser_input inp = clane::make_parser_input("blah blah blah", cons_str, rem_str);
			parser.reset();
			bool parser_stat = parser.parse(inp.str.get() + inp.offset, inp.size);
			check_false(parser_stat); // must not have completed
			check_false(parser); // must have failed
			checker();
		}

		// character-by-character:
		{
			parser.reset();
			std::vector<char> inp(cons_str, cons_str + strlen(cons_str));
			for (size_t i = 0; i < inp.size(); ++i) {
				// empty buffer:
				bool parser_stat = parser.parse(inp.data() + i, 0);
				check_false(parser_stat); // must not have completed
				check_true(parser); // no error yet
				// one-byte buffer:
				parser_stat = parser.parse(inp.data() + i, 1);
				check_false(parser_stat); // must not have completed
				if (i + 1 < inp.size()) {
					check_true(parser); // no error yet
				} else {
					check_false(parser); // error
					checker();
				}
			}
		}
	}

	template <class Parser>
	void check_parse_ok_(char const *sname, int sline, Parser &parser, size_t len_limit,
			char const *cons_str, char const *rem_str, std::function<void()> const &checker) {

		std::cerr << "trying " << sname << ":" << sline << std::endl;
		parser.len_limit = len_limit;

		// full string at once:
		{
			parser.reset();
			clane::parser_input inp = clane::make_parser_input("blah blah blah", cons_str, rem_str);
			bool parser_stat = parser.parse(inp.str.get() + inp.offset, inp.size);
			check_true(parser_stat); // must have completed with success
			checker();
		}

		// char-by-character:
		{
			parser.reset();
			std::vector<char> inp(cons_str, cons_str + strlen(cons_str));
			for (size_t i = 0; i < inp.size(); ++i) {
				// empty buffer:
				bool parser_stat = parser.parse(inp.data() + i, 0);
				check_false(parser_stat); // must not have completed
				check_true(parser);
				// one-byte buffer:
				parser_stat = parser.parse(inp.data() + i, 1);
				if (i + 1 < inp.size()) {
					check_false(parser_stat);
					check_true(parser);
				} else {
					check_true(parser_stat);
					check_eq(strlen(cons_str), parser.parse_size());
					checker();
				}
			}
		}
	}

}

#endif // #ifndef CLANE_PARSE_H
