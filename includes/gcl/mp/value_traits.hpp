#pragma once

#include <gcl/mp/concepts.hpp>
#include <concepts>
#include <utility>
#include <tuple>
#include <type_traits>

namespace gcl::mp::value_traits
{
    template <auto... values>
    struct sequence {
        constexpr static auto size = sizeof...(values);
    };

    template <auto ... values>
    struct equal : std::bool_constant<(sizeof...(values) == 0)>{};
    template <auto V, auto ... Vs>
    requires (std::equality_comparable_with<decltype(V), decltype(Vs)> and ...)
    struct equal<V, Vs...> : std::bool_constant<((V == Vs) and ...)>{};
    template <auto ... values>
    constexpr inline bool equal_v = equal<values...>::value;

    template <auto ... values>
    struct not_equal : std::negation<equal<values...>>{};
    template <auto ... values>
    constexpr inline bool not_equal_v = not_equal<values...>::value;
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

#if defined(GCL_ENABLE_COMPILE_TIME_TESTS)
namespace gcl::mp::value_traits::tests::equal
{
    static_assert(equal_v<>);
    static_assert(equal_v<42>);
    static_assert(equal_v<42, char{42}>);
    static_assert(equal_v<42, char{42}, std::size_t{42}>);

    static_assert(not not_equal_v<>);
    static_assert(not not_equal_v<42>);
    static_assert(not not_equal_v<42, char{42}>);
    static_assert(not not_equal_v<42, char{42}, std::size_t{42}>);
}

#include <gcl/mp/preprocessor.hpp>
#include <limits>
namespace gcl::mp::value_traits::tests
{
    [[maybe_unused]] void bit_size_v()
    {
        if constexpr (gcl::mp::preprocessor::plateform::is_x64)
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
    [[maybe_unused]] void values_count()
    {
        if constexpr (gcl::mp::preprocessor::plateform::is_x64)
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
#endif