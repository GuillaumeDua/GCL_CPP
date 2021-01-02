#pragma once

#include <gcl/mp/type_traits.hpp>

namespace gcl::mp
{
    template <typename T, typename = std::enable_if_t<gcl::mp::type_traits::is_template_v<T>>>
    struct pack_traits {
        using type = T;

        // Limitation : Clang 11.0.0 does not support "Lambdas in unevaluated contexts" (P0315R4)
        using arguments = std::remove_reference_t<decltype([]<template <typename...> typename Type, typename... Ts>(
            Type<Ts...>) constexpr { return std::tuple<Ts...>{}; }(std::declval<T>()))>;

        template <template <class...> class template_type>
        static inline constexpr auto is_instance_of_v = type_traits::is_instance_of_v<T, template_type>;

        // static constexpr inline auto size =
    };
}