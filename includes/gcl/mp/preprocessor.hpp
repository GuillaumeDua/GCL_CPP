#pragma once

#if defined(_WIN32) or defined(_WIN64)
	#if _WIN64
		#define TARGET_IS_x64 true
		#define TARGET_IS_x86 false
	#else
		#define TARGET_IS_x64 false
		#define TARGET_IS_x86 true
	#endif
#elif defined(__GNUC__)
	#if __x86_64__ || __ppc64__
		#define TARGET_IS_x64 true
		#define TARGET_IS_x86 false
	#else
		#define TARGET_IS_x64 false
		#define TARGET_IS_x86 true
	#endif
#else
	#error "Plateform not supported"
#endif

namespace gcl::mp::preprocessor::plateform
{
    constexpr static auto is_x64 = TARGET_IS_x64;
    constexpr static auto is_x86 = TARGET_IS_x86;

    static_assert(is_x64 or is_x86);
    static_assert(not(is_x64 and is_x86));
}
namespace gcl::mp::preprocessor::compiler
{
    constexpr auto is_clang =
#if defined(__clang__)
        true
#else
        false
#endif
        ;
    constexpr auto is_clang_cl =
#if defined(_MSC_VER) and defined(__clang__)
        true
#else
        false
#endif
        ;
    constexpr auto is_msvc_cl =
#if defined(_MSC_VER) and not defined(__clang__)
        true
#else
        false
#endif
        ;
    constexpr auto is_gcc =
#if defined(GCC_VERSION)
        true
#else
        false
#endif
        ;
}
namespace gcl::mp::preprocessor
{
    constexpr auto gcl_enable_compile_time_tests =
#if defined(GCL_ENABLE_COMPILE_TIME_TESTS)
        true
#else
        false
#endif
        ;
    constexpr auto gcl_enable_runtime_tests =
#if defined(GCL_ENABLE_RUNTIME_TESTS)
        true
#else
        false
#endif
        ;
}