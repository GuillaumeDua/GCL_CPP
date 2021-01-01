#pragma once

#include <tuple>

namespace gcl::tuple_utils
{
    // Limitation : Clang 11.0.0 does not support "Lambdas in unevaluated contexts" (P0315R4)
    template <typename T>
    using type_parameters_as_tuple_t = decltype([]<template <typename...> typename Type, typename... Ts>(Type<Ts...>) constexpr {
        return std::tuple<Ts...>{};
    }(std::declval<T>()));

    // template <std::size_t N, typename T>
    // using type_at_t = std::tuple_element_t<N, typename args_as_tuple_t<T>>; // arg 2 invalid ?
}