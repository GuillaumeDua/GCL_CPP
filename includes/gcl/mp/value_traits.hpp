#pragma once

#include <gcl/mp/concepts.hpp>

namespace gcl::mp::value_traits
{
    template <auto... values>
    requires(std::equality_comparable_with<
             decltype(std::get<0>(std::tuple{values...})),
             decltype(values)>&&...) constexpr static auto equal_v = []() consteval
    {
        return ((values == std::get<0>(std::tuple{values...})) && ...);
    }
    ();
    template <auto... values>
    constexpr static auto not_equal_v = not equal_v<values...>;
}

namespace gcl::mp::value_traits::tests::equal
{
    static_assert(equal_v<>);
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