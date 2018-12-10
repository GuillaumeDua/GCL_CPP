#pragma once

#include <type_traits>

namespace gcl::type_traits
{
	template<class, class T, typename... Args>
	struct is_brace_constructible : std::false_type
	{};
	template<class T, typename... Args>
	struct is_brace_constructible<std::void_t<decltype(T{ std::declval<Args>()... })>, T, Args...>
		: std::true_type
	{};

	template<class T, typename... Args>
	constexpr static inline bool is_brace_constructible_v = is_brace_constructible<std::void_t<>, T, Args...>::value;
}