add_executable(gcl_test_binary
		${CMAKE_CURRENT_SOURCE_DIR}/Main.cpp
	)
set_target_properties(gcl_test_binary
PROPERTIES
	CXX_STANDARD_REQUIRED ON
	CXX_STANDARD 20
	CXX_EXTENSIONS OFF
)
add_dependencies(gcl_test_binary gcl_cpp)
target_link_libraries(gcl_test_binary
	PRIVATE
		gcl_cpp
)
