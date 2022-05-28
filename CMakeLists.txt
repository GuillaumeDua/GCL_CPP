cmake_minimum_required(VERSION 3.8)
project(GCL VERSION 2.0 LANGUAGES CXX)

option(${PROJECT_NAME}_ENABLE_COMPILE_TIME_TESTS "${PROJECT_NAME} : Build compile-time tests ?" OFF)
message(STATUS "${PROJECT_NAME} -- ${PROJECT_NAME}_ENABLE_COMPILE_TIME_TESTS is ${${PROJECT_NAME}_ENABLE_COMPILE_TIME_TESTS}")
if (${${PROJECT_NAME}_ENABLE_COMPILE_TIME_TESTS})
	add_compile_definitions(${PROJECT_NAME}_ENABLE_COMPILE_TIME_TESTS)
endif()

add_library(gcl_cpp INTERFACE)
target_include_directories(gcl_cpp INTERFACE
	${CMAKE_CURRENT_SOURCE_DIR}/includes
)
target_compile_features(gcl_cpp INTERFACE
	cxx_std_20
	# C++11
	cxx_lambdas
	cxx_noexcept
	cxx_auto_type
	cxx_constexpr
	cxx_static_assert
	cxx_alias_templates
	cxx_rvalue_references
	# C++14
	cxx_decltype_auto
	cxx_generic_lambdas
	cxx_variable_templates
	cxx_variadic_templates
	cxx_return_type_deduction
	cxx_aggregate_default_initializers
)

option(${PROJECT_NAME}_BUILD_TEST "${PROJECT_NAME} : Build tests ?" OFF)
message(STATUS "${PROJECT_NAME} -- ${PROJECT_NAME}_BUILD_TEST is ${${PROJECT_NAME}_BUILD_TEST}")
if (${PROJECT_NAME}_BUILD_TEST)
	message(STATUS "${PROJECT_NAME} -- ${PROJECT_NAME}_BUILD_TEST -- forcing ${PROJECT_NAME}_ENABLE_COMPILE_TIME_TESTS to ON")
	add_compile_definitions(${PROJECT_NAME}_ENABLE_COMPILE_TIME_TESTS)
	message(STATUS "${PROJECT_NAME} -- ${PROJECT_NAME}_BUILD_TEST -- forcing ${PROJECT_NAME}_ENABLE_RUNTIME_TESTS to ON")
	add_compile_definitions(${PROJECT_NAME}_ENABLE_RUNTIME_TESTS)
	add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/tests)
endif()

# todo : include2dot
# todo : dependency graph
#	cmake SRC_DIR --graphviz=xxx.dot
#	dot -Tpng xxx.dot xxx.pgf
