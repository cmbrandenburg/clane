// vim: set noet:

#include "../http_header.h"
#include "../../check/check.h"

int main() {
	using clane::http::header_map;
	header_map m1, m2;

	// same with same case:
	m1.insert(header_map::value_type("alpha", "bravo"));
	m2.insert(header_map::value_type("alpha", "bravo"));
	check_true(m1 == m2);
	check_true(m2 == m1);
	check_false(m1 != m2);
	check_false(m2 != m1);
	check_false(m1 < m2);
	check_false(m2 < m1);
	check_true(m1 <= m2);
	check_true(m2 <= m1);
	check_false(m1 > m2);
	check_false(m2 > m1);
	check_true(m1 >= m2);
	check_true(m2 >= m1);

	// same with different case:
	m2.clear();
	m2.insert(header_map::value_type("Alpha", "bravo"));
	check_true(m1 == m2);
	check_true(m2 == m1);
	check_false(m1 != m2);
	check_false(m2 != m1);
	check_false(m1 < m2);
	check_false(m2 < m1);
	check_true(m1 <= m2);
	check_true(m2 <= m1);
	check_false(m1 > m2);
	check_false(m2 > m1);
	check_true(m1 >= m2);
	check_true(m2 >= m1);

	// same names but different values:
	m2.clear();
	m2.insert(header_map::value_type("alpha", "charlie"));
	check_false(m1 == m2);
	check_false(m2 == m1);
	check_true(m1 != m2);
	check_true(m2 != m1);
	check_true(m1 < m2);
	check_false(m2 < m1);
	check_true(m1 <= m2);
	check_false(m2 <= m1);
	check_false(m1 > m2);
	check_true(m2 > m1);
	check_false(m1 >= m2);
	check_true(m2 >= m1);

	// different:
	m2.clear();
	m2.insert(header_map::value_type("delta", "bravo"));
	check_false(m1 == m2);
	check_false(m2 == m1);
	check_true(m1 != m2);
	check_true(m2 != m1);
	check_true(m1 < m2);
	check_false(m2 < m1);
	check_true(m1 <= m2);
	check_false(m2 <= m1);
	check_false(m1 > m2);
	check_true(m2 > m1);
	check_false(m1 >= m2);
	check_true(m2 >= m1);

	return 0;
}

