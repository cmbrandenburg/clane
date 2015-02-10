macro(make_cxx_test name)
	add_executable(check_${name} check_${name}.cpp)
	target_link_libraries(check_${name} clane)
	add_test(${name} ./check_${name})
endmacro()

make_cxx_test(parse_line)
make_cxx_test(server)
