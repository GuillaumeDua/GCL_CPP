#pragma once

#include <concepts>

namespace gcl::mp::value_traits
{
    template <auto... values>
    constexpr static auto equal_v = []() consteval
    {
        static_assert(sizeof...(values) > 0, "gcl::mp::value_traits::equal_v : no arguments");
        constexpr auto first_value = std::get<0>(std::tuple{values...});
        static_assert(
            (std::equality_comparable_with<decltype(values), decltype(first_value)> && ...),
            "gcl::mp::value_traits::equal_v : cannot compare values");
        return ((values == first_value) && ...);
    }
    ();
    template <auto... values>
    constexpr static auto not_equal_v = not equal_v<values...>;
}

namespace gcl::mp::value_traits::tests::equal
{
    static_assert(equal_v<true>);
    static_assert(equal_v<true, true>);
    static_assert(equal_v<true, true, true>);
    static_assert(equal_v<true, true, true, true>);

    static_assert(not not_equal_v<true>);
    static_assert(not_equal_v<true, false>);
    static_assert(not_equal_v<false, true>);
    static_assert(not_equal_v<false, true, true>);
    static_assert(not_equal_v<true, false, true>);
    static_assert(not_equal_v<true, true, false>);
    static_assert(not_equal_v<false, true, true, true>);
    static_assert(not_equal_v<true, false, true, true>);
    static_assert(not_equal_v<true, true, false, true>);
    static_assert(not_equal_v<true, true, true, false>);
}