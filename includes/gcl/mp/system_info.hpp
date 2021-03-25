#pragma once

#if defined(_WIN32) or defined(_WIN64)
	#if _WIN64
		#define TARGET_IS_x64 true
		#define TARGET_IS_x32 false
	#else
		#define TARGET_IS_x64 false
		#define TARGET_IS_x32 true
	#endif
#elif defined(__GNUC__)
	#if __x86_64__ || __ppc64__
		#define TARGET_IS_x64 true
		#define TARGET_IS_x32 false
	#else
		#define TARGET_IS_x64 false
		#define TARGET_IS_x32 true
	#endif
#endif

namespace gcl::mp::system_info
{
    constexpr static auto is_x64 = TARGET_IS_x64;
    constexpr static auto is_x32 = TARGET_IS_x32;

    static_assert(is_x64 or is_x32);
    static_assert(not(is_x64 and is_x32));
}