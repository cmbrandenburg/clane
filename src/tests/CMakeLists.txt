add_executable(check_server check_server.cpp)
target_link_libraries(check_server clane)
add_test(server ./check_server)
