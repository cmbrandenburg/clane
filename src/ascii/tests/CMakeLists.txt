macro(make_cxx_test name)
	add_executable(check_ascii_${name} check_ascii_${name}.cpp)
	add_test(ascii_${name} ./check_ascii_${name})
endmacro()

make_cxx_test(has_prefix)
make_cxx_test(hexch_to_int)
make_cxx_test(int_to_hexch)
