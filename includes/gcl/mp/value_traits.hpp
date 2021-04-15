#pragma once

#include <gcl/mp/concepts.hpp>
#include <concepts>

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
    // constexpr inline auto equal_v = []() consteval
    // {
    //     return ((values == first_value) && ...);
    // }
    // ();
    template <auto... values>
    constexpr inline auto not_equal_v = not equal_v<values...>;
}
#include <cstdint>
#include <climits>
namespace gcl::mp::value_traits
{
    template <typename T>
    constexpr inline std::size_t bit_size_v = sizeof(T) * CHAR_BIT;
    // sizeof(bool) is implementation-defined,
    // but here we expect - thus, define - that a boolean is 1 bit-long
    template <>
    constexpr inline std::size_t bit_size_v<bool> = 1;

    template <typename T>
    requires(bit_size_v<std::uintmax_t> > bit_size_v<T>) constexpr static auto values_count =
        (std::uintmax_t{1} << bit_size_v<T>);
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
#include <limits>
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
    static void values_count()
    {
        if constexpr (gcl::mp::system_info::is_x64)
        { // might be wrong, depending on the target/plateform
            static_assert(gcl::mp::value_traits::values_count<bool> == 2);
            static_assert(gcl::mp::value_traits::values_count<char> - 1 == std::numeric_limits<unsigned char>::max());
            static_assert(gcl::mp::value_traits::values_count<int8_t> - 1 == std::numeric_limits<uint8_t>::max());
            static_assert(gcl::mp::value_traits::values_count<uint8_t> - 1 == std::numeric_limits<uint8_t>::max());
            static_assert(gcl::mp::value_traits::values_count<int16_t> - 1 == std::numeric_limits<uint16_t>::max());
            static_assert(gcl::mp::value_traits::values_count<uint16_t> - 1 == std::numeric_limits<uint16_t>::max());
            static_assert(gcl::mp::value_traits::values_count<int32_t> - 1 == std::numeric_limits<uint32_t>::max());
            static_assert(gcl::mp::value_traits::values_count<uint32_t> - 1 == std::numeric_limits<uint32_t>::max());

            // violate `values_count` constraint :
            //  C4293: '<<': shift count negative or too big, undefined behavior
            /*static_assert(gcl::mp::value_traits::values_count<int64_t> - 1 == std::numeric_limits<uint64_t>::max());
            static_assert(gcl::mp::value_traits::values_count<uint64_t> - 1 == std::numeric_limits<uint64_t>::max());*/
        }
    }
}