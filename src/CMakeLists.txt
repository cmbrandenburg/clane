include_directories(BEFORE "${CMAKE_CURRENT_SOURCE_DIR}")
include_directories(BEFORE "${CMAKE_SOURCE_DIR}/include")
include_directories(${Boost_INCLUDE_DIRS})
include_directories(${OPENSSL_INCLUDE_DIR})
list(APPEND CMAKE_CXX_FLAGS "-std=c++14 -Wall -Werror")
add_subdirectory(check)
add_subdirectory(http)
add_subdirectory(uri)
add_library(clane clane_main.cpp)
target_link_libraries(clane
	clane-http
	clane-uri)
