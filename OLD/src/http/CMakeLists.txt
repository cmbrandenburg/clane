add_library(clane-http STATIC
	clane_http_message.cxx
	clane_http_server.cxx)
target_link_libraries(clane-http
	clane-uri
	${Boost_LIBRARIES}
	${OPENSSL_LIBRARIES}
	${CMAKE_THREAD_LIBS_INIT})
add_subdirectory(tests)
