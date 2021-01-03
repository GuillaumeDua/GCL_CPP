#pragma once

#include <type_traits>
namespace gcl::mp::type_traits
{
    template <class T>
    struct is_template : std::false_type {};
    template <class... T_args, template <class...> class T>
    struct is_template<T<T_args...>> : std::true_type {};
    template <class T>
    static inline constexpr auto is_template_v = is_template<T>::value;

    template <class T_concrete, template <class...> class T>
    struct is_instance_of : std::false_type {};
    template <template <class...> class T, class... T_args>
    struct is_instance_of<T<T_args...>, T> : std::true_type {};

    template <class T_concrete, template <class...> class T>
    static inline constexpr auto is_instance_of_v = is_instance_of<T_concrete, T>::value;

    template <class T, typename... Args>
    class is_brace_constructible {
        template <typename /*= void*/, typename U, typename... U_args>
        struct impl : std::false_type {};
        template <typename U, typename... U_args>
        struct impl<std::void_t<decltype(U{std::declval<U_args>()...})>, U, U_args...> : std::true_type {};

      public:
        constexpr inline static auto value = impl<std::void_t<>, T, Args...>::value;
    };
    template <class T, typename... Args>
    constexpr static inline bool is_brace_constructible_v = is_brace_constructible<T, Args...>::value;
}

#include <tuple>
namespace gcl::mp::type_traits
{
    // Limitations
    //  Clang   11.0.0  : does not support "Lambdas in unevaluated contexts" (P0315R4)
    //  MsVC    19.28   : "error C2057: expected constant expression" in consteval context
    //  (static_assert<std::is_same_v<>>) GCC     10.2    : OK
    // template <
    //     typename T,
    //     template <typename...> class PackType = std::tuple,
    //     typename = std::enable_if_t<type_traits::is_template_v<T>>>
    // using pack_arguments = std::remove_reference_t<decltype([]<template <typename...> typename Type, typename... Ts>(
    //     Type<Ts...>) constexpr { return PackType<Ts...>{}; }(std::declval<T>()))>;
    template <
        typename T,
        template <typename...> class PackType = std::tuple,
        typename = std::enable_if_t<type_traits::is_template_v<T>>>
    class pack_arguments {
        template <template <typename...> typename Type, typename... Ts>
        static auto impl(Type<Ts...>)
        {   // type deducer
            return PackType<Ts...>{};
        }

      public:
        using type = decltype(impl(std::declval<T>()));
    };
    template <
        typename T,
        template <typename...> class PackType = std::tuple,
        typename = std::enable_if_t<type_traits::is_template_v<T>>>
    using pack_arguments_t = typename pack_arguments<T, PackType>::type;
}

namespace gcl::mp::type_traits::tests::pack
{
    template <typename... Ts>
    struct pack_type {};

    using toto = typename gcl::mp::type_traits::pack_arguments_t<pack_type<int, double, float>, std::tuple>;
    using titi = typename gcl::mp::type_traits::pack_arguments_t<pack_type<int, double, float>>;
    static_assert(std::is_same_v<titi, toto>);
    static_assert(std::is_same_v<titi, std::tuple<int, double, float>>);
    static_assert(
        std::is_same_v<pack_type<int, double, float>, gcl::mp::type_traits::pack_arguments_t<titi, pack_type>>);
}
namespace gcl::mp::type_traits::tests::is_brace_constructible_v
{
    struct toto {
        int i;
    };
    struct titi {
        explicit titi(int) {}
    };

    static_assert(type_traits::is_brace_constructible_v<toto>);
    static_assert(type_traits::is_brace_constructible_v<toto, int>);
    static_assert(type_traits::is_brace_constructible_v<toto, char>);
    static_assert(not type_traits::is_brace_constructible_v<toto, char*>);
    static_assert(not type_traits::is_brace_constructible_v<titi>);
    static_assert(type_traits::is_brace_constructible_v<titi, int>);
    static_assert(type_traits::is_brace_constructible_v<titi, char>);
    static_assert(not type_traits::is_brace_constructible_v<titi, char*>);
}