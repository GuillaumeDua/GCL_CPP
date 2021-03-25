#pragma once

#include <gcl/mp/concepts.hpp>

namespace gcl::mp::value_traits
{
    template <auto... values>
    struct sequence {
        constexpr static auto size = sizeof...(values);
    };

    template <auto... values>
    requires(std::equality_comparable_with<
             decltype(std::get<0>(std::tuple{values...})),
             decltype(values)>&&...) constexpr static auto equal_v = []() consteval
    {
        return ((values == std::get<0>(std::tuple{values...})) && ...);
    }
    ();
    // Only works for Clang yet, since 11.0.0 
    // template <auto first_value = int{}, std::equality_comparable_with<decltype(first_value)> auto... values>
    // constexpr static auto equal_v = []() consteval
    // {
    //     return ((values == first_value) && ...);
    // }
    // ();
    template <auto... values>
    constexpr static auto not_equal_v = not equal_v<values...>;
}
#include <cstdint>
namespace gcl::mp::value_traits
{
    template <typename T>
    constexpr std::size_t bit_size_v = sizeof(T) * CHAR_BIT;
    // sizeof(bool) is implementation-defined,
    // but here we expect - thus, define - that a boolean is 1 bit-long
    template <>
    constexpr std::size_t bit_size_v<bool> = 1;
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

#include <gcl/mp/system_info.hpp>
namespace gcl::mp::value_traits::tests
{
    static void bit_size_v()
    {
        if constexpr (gcl::mp::system_info::is_x64)
        {   // might be wrong, depending on the target/plateform
            static_assert(gcl::mp::value_traits::bit_size_v<bool> == 1);
            static_assert(gcl::mp::value_traits::bit_size_v<char> == 8);
            static_assert(gcl::mp::value_traits::bit_size_v<int8_t> == 8);
            static_assert(gcl::mp::value_traits::bit_size_v<uint8_t> == 8);
            static_assert(gcl::mp::value_traits::bit_size_v<int16_t> == 16);
            static_assert(gcl::mp::value_traits::bit_size_v<uint16_t> == 16);
            static_assert(gcl::mp::value_traits::bit_size_v<int32_t> == 32);
            static_assert(gcl::mp::value_traits::bit_size_v<uint32_t> == 32);
            static_assert(gcl::mp::value_traits::bit_size_v<int64_t> == 64);
            static_assert(gcl::mp::value_traits::bit_size_v<uint64_t> == 64);
        }
    }
}