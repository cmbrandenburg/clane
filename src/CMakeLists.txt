list(APPEND CMAKE_CXX_FLAGS "-std=c++14 -Wall -Werror")
include_directories(BEFORE "${CMAKE_SOURCE_DIR}/include")
include_directories(${Boost_INCLUDE_DIRS})
include_directories(${OPENSSL_INCLUDE_DIR})
add_library(clane
	clane_http_server.cpp)
target_link_libraries(clane
	${Boost_LIBRARIES}
	${OPENSSL_LIBRARIES}
	${CMAKE_THREAD_LIBS_INIT})
add_subdirectory(tests)
