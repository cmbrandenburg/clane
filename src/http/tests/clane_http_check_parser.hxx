// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// vim: set noet:

#ifndef CLANE_HTTP_CHECK_PARSER_HXX
#define CLANE_HTTP_CHECK_PARSER_HXX

#include "check/clane_check.hxx"
#include <string>

// Tests an incremental parser for success by parsing an input string two times:
// one time all at once and again byte-by-byte.
template <typename Parser, typename PostCheck> class ok_checker {
	Parser &    m_parser;
	std::string m_good;
	PostCheck   m_post_check;

public:
	ok_checker(Parser &parser, char const *good, PostCheck &&chk):
		m_parser(parser),
		m_good{good},
		m_post_check{std::forward<PostCheck>(chk)}
	{}

	ok_checker(ok_checker const &) = default;
	ok_checker(ok_checker &&) = default;
	ok_checker &operator=(ok_checker const &) = default;
	ok_checker &operator=(ok_checker &&) = default;

	void operator()() {

		// parse the entire input string all at once:
		m_parser.reset();
		std::string q{m_good};
		q.append("EXTRA");
		auto n = m_parser.parse(q.data(), q.size());
		check(!m_parser.bad());
		check(m_parser.fin());
		check(n == m_good.size());
		m_post_check();

		// parse the input one character at a time:
		m_parser.reset();
		for (std::size_t i = 0; i+1 < m_good.size(); i++) {
			n = m_parser.parse(&m_good[i], 1);
			check(!m_parser.bad());
			check(!m_parser.fin());
			check(1 == n);
			n = m_parser.parse("", 0);
			check(!m_parser.bad());
			check(!m_parser.fin());
			check(0 == n);
		}
		n = m_parser.parse(&m_good[m_good.size()-1], 1);
		check(!m_parser.bad());
		check(m_parser.fin());
		check(1 == n);
		m_post_check();
	}
};

template <typename Parser, typename PostCheck>
ok_checker<Parser, PostCheck> make_ok_checker(Parser &parser, char const *good, PostCheck &&chk) {
	return ok_checker<Parser, PostCheck>{parser, good, std::forward<PostCheck>(chk)};
}

// Tests an incremental parser for error by parsing an input string two times:
// one time all at once and again byte-by-byte.
template <typename Parser, typename PostCheck> class nok_checker {
	Parser &    m_parser;
	std::string m_good;
	std::string m_bad;
	PostCheck   m_post_check;

public:
	nok_checker(Parser &parser, char const *good, char const *bad, PostCheck &&chk):
		m_parser(parser),
		m_good{good},
		m_bad{bad},
		m_post_check{std::forward<PostCheck>(chk)}
	{}

	nok_checker(nok_checker const &) = default;
	nok_checker(nok_checker &&) = default;
	nok_checker &operator=(nok_checker const &) = default;
	nok_checker &operator=(nok_checker &&) = default;

	void operator()() {
		// parse the entire input string all at once:
		m_parser.reset();
		std::string q{m_good};
		q.append(m_bad);
		auto n = m_parser.parse(q.data(), q.size());
		check(m_parser.bad());
		check(0 == n);
		m_post_check();

		// parse the input one character at a time:
		m_parser.reset();
		for (std::size_t i = 0; i < m_good.size(); ++i) {
			n = m_parser.parse(&m_good[i], 1);
			check(!m_parser.bad());
			check(!m_parser.fin());
			check(1 == n);
			n = m_parser.parse("", 0);
			check(!m_parser.bad());
			check(!m_parser.fin());
			check(0 == n);
		}
		n = m_parser.parse(&m_bad[0], 1);
		check(m_parser.bad());
		check(0 == n);
		m_post_check();
	}
};

template <typename Parser, typename PostCheck>
nok_checker<Parser, PostCheck> make_nok_checker(Parser &parser, char const *good, char const *bad, PostCheck &&chk) {
	return nok_checker<Parser, PostCheck>{parser, good, bad, std::forward<PostCheck>(chk)};
}

#endif // #ifndef CLANE_HTTP_CHECK_PARSER_HXX
