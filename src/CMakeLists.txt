include_directories(${Boost_INCLUDE_DIRS})
add_library(clane
	clane_http.cpp)
target_link_libraries(clane
	${Boost_LIBRARIES})
