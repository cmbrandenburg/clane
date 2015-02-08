if(DOXYGEN_FOUND)
add_custom_target(
	api-doc
	ALL
	COMMAND pwd
	COMMAND doxygen "${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile")
set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES html)
endif()
