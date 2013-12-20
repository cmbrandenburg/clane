// vim: set noet:

#include "parse.h"

namespace clane {

	parser_input make_parser_input(char const *prefix, char const *cons_str, char const *rem_str) {
		parser_input cons_inp{};
		cons_inp.cap = strlen(prefix) + strlen(cons_str) + strlen(rem_str);
		cons_inp.str.reset(new char[cons_inp.cap]);
		memcpy(cons_inp.str.get(), prefix, strlen(prefix));
		memcpy(cons_inp.str.get() + strlen(prefix), cons_str, strlen(cons_str));
		memcpy(cons_inp.str.get() + strlen(prefix) + strlen(cons_str), rem_str, strlen(rem_str));
		cons_inp.offset = strlen(prefix);
		cons_inp.size = strlen(cons_str) + strlen(rem_str);
		return cons_inp;
	}

}

