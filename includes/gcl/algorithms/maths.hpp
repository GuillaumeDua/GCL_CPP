#pragma once

#include <type_traits>
#include <cstdint>
#include <limits>

namespace gcl::algorithms::maths
{
    template <typename T>
    constexpr auto distance(T x, T y)
    {
        using value_type = std::conditional_t<std::is_signed_v<T>, std::intmax_t, std::uintmax_t>;
        return y >= x ? (value_type{y} - value_type{x}) : (value_type{x} - value_type{y});
    }
    
    // std::abs is not constexpr yet
    template <typename T>
    requires(
        (std::is_signed_v<T> and std::numeric_limits<T>::min() >= std::numeric_limits<std::intmax_t>::min() and
         std::numeric_limits<T>::max() <= std::numeric_limits<std::intmax_t>::max()) or
        not std::is_signed_v<T>) constexpr inline auto abs(T arg) noexcept
    {
        if constexpr (std::is_signed_v<T>)
        {
            std::intmax_t value{arg};
            return value < 0 ? -value : value;
        }
        else
        {
            return arg;
        }
    }
}

namespace gcl::algorithms::tests::maths
{
    static_assert(gcl::algorithms::maths::distance(0, 0) == 0);
    static_assert(gcl::algorithms::maths::distance(-1, 1) == 2);
    static_assert(gcl::algorithms::maths::distance(1, -1) == 2);
    constexpr unsigned char uchar_zero{0};
    static_assert(
        gcl::algorithms::maths::distance(uchar_zero, std::numeric_limits<unsigned char>::max()) ==
        std::numeric_limits<unsigned char>::max());

    static_assert(gcl::algorithms::maths::abs(-1) == 1);
    static_assert(gcl::algorithms::maths::abs(1) == 1);
}