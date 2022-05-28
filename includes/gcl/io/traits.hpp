#pragma once

#include <type_traits>
#include <iostream>

namespace gcl::io::type_traits::detect
{
    template <typename T, typename = void>
    struct has_ostream_shift_operator : std::false_type {};
    template <typename T>
    struct has_ostream_shift_operator<T, std::void_t<decltype(std::declval<std::ostream&>() << std::declval<T>())>>
        : std::true_type {};

    template <typename T, typename = void>
    struct has_istream_shift_operator : std::false_type {};
    template <typename T>
    struct has_istream_shift_operator<T, std::void_t<decltype(std::declval<std::istream&>() >> std::declval<T&&>())>>
        : std::true_type {};
}