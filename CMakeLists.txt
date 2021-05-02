cmake_minimum_required(VERSION 3.8)
project(GCL VERSION 2.0 LANGUAGES CXX)

add_library(gcl_cpp INTERFACE)
target_include_directories(gcl_cpp INTERFACE
	${CMAKE_CURRENT_SOURCE_DIR}/includes
)
target_compile_features(gcl_cpp INTERFACE
	cxx_std_20
	cxx_noexcept
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

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" AND
	"${CMAKE_CXX_SIMULATE_ID}" STREQUAL "MSVC")
	set(CXX_COMPILER_IS_CLANG_CL true)
else()
	set(CXX_COMPILER_IS_CLANG_CL false)
endif()

option(${PROJECT_NAME}_USE_CLANG_TIDY "${PROJECT_NAME} : Run clang-tidy checks ?" OFF)
message(STATUS "${PROJECT_NAME}_USE_CLANG_TIDY : ${${PROJECT_NAME}_USE_CLANG_TIDY}")
if (${PROJECT_NAME}_USE_CLANG_TIDY)
	set(CMAKE_CXX_CLANG_TIDY clang-tidy -checks=*) #,-readability=*
	# fix clang-tidy exception miss-detection using Clang-CL
	target_compile_options(gcl_cpp INTERFACE
		$<IF:$<OR:$<CXX_COMPILER_ID:MSVC>,$<BOOL:${CXX_COMPILER_IS_CLANG_CL}>>,-EHsc,-fexceptions>
	)
endif()

option(${PROJECT_NAME}_BUILD_TEST "${PROJECT_NAME} : Build tests ?" OFF)
message(STATUS "${PROJECT_NAME}_BUILD_TEST : ${${PROJECT_NAME}_BUILD_TEST}")
if (${PROJECT_NAME}_BUILD_TEST)
	add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/tests)
endif()

# todo : include2dot
# todo : dependency graph
#	cmake SRC_DIR --graphviz=xxx.dot
#	dot -Tpng xxx.dot xxx.pgf
