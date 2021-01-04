#pragma once

#include <gcl/mp/type_traits.hpp>
#include <tuple>

namespace gcl::mp::type_traits
{
    // Limitations : compilers support
    //  Clang   11.0.0  : does not support "Lambdas in unevaluated contexts" (P0315R4)
    //  MsVC    19.28   : "error C2057: expected constant expression" in consteval context
    //                    (like static_assert<std::is_same_v<>>)
    //  GCC     10.2    : OK
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

    template <typename T, typename U>
    struct concatenate;
    template <template <typename...> typename T, typename ... Ts, typename ... Us>
    struct concatenate<T<Ts...>, T<Us...>>
    {
        using type = T<Ts..., Us...>;
    };
    template <typename T, typename U>
    using concatenate_t = typename concatenate<T,U>::type;

    template <std::size_t N, typename... Ts>
    using type_at = typename std::tuple_element<N, std::tuple<Ts...>>;
    template <std::size_t N, typename... Ts>
    using type_at_t = typename type_at<N, Ts...>::type;

    // todo : index_of

    template <typename T, typename... Ts>
    using contains = std::disjunction<std::is_same<T, Ts>...>;
    template <typename T, typename... Ts>
    static constexpr inline auto contains_v = contains<T, Ts...>::value;

    template <template <typename> typename trait, typename... Ts>
    class filters {
        template <typename T>
        using impl = std::conditional_t<trait<T>::value, std::tuple<T>, std::tuple<>>;

      public:
        using type = decltype(std::tuple_cat(impl<Ts>{}...));
    };
    template <template <typename> typename trait, typename... Ts>
    using filters_t = typename filters<trait, Ts...>::type;
}
namespace gcl::mp
{
    template <typename T, typename = std::enable_if_t<gcl::mp::type_traits::is_template_v<T>>>
    struct pack_traits;
    template <template <typename...> typename T, typename... Ts>
    struct pack_traits<T<Ts...>> {
        using type = T<Ts...>;
        using arguments = type_traits::pack_arguments_t<type>;
        template <template <class...> class template_type>
        using unpack_as = type_traits::pack_arguments_t<type, template_type>;
        template <size_t N>
        using type_at = type_traits::type_at_t<N, Ts...>; // typename std::tuple_element<N, arguments>::type;

        static constexpr inline auto size = std::tuple_size_v<arguments>;
        template <typename U>
        static constexpr inline auto contains = type_traits::contains_v<U, Ts...>;
        template <template <class...> class template_type>
        static inline constexpr auto is_instance_of_v = type_traits::is_instance_of_v<type, template_type>;

        template <template <typename> typename trait>
        using satisfy_trait_t = std::conjunction<trait<Ts>...>;
        template <template <typename> typename trait>
        constexpr static inline bool satisfy_trait_v = satisfy_trait_t<trait>::value; //(trait<Ts>::value && ...)

        // template <template <typename> typename trait>
        // using filters = pack_traits<typename gcl::mp::type_traits::filters_t<trait, Ts...>>::unpack_as<T>;
    };
    template <typename... Ts>
    struct pack_type { // empty type that has variadic template-type parameters
                       // use this instead of std::tuple to pack template-type parameters,
                       // if your optimization level does not skip unused variables for some reasons
    };
    template <typename... Ts>
    struct super : Ts... {};
    template <template <typename...> class base_type, typename... Ts>
    struct partial {
        // differs type instanciation with partial template-type parameters
        template <typename... Us, typename = std::enable_if_t<sizeof...(Us) >= 1>>
        using type = base_type<Ts..., Us...>;
    };
}

namespace gcl::mp::type_traits::tests::pack_arguments
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
namespace gcl::mp::type_traits::tests
{
    static_assert(std::is_same_v<gcl::mp::type_traits::type_at_t<2, char, bool, int, float>, int>);
    static_assert(gcl::mp::type_traits::contains_v<int, char, bool, int, float>);

    static_assert(gcl::mp::partial<std::is_same, int>::type<int>::value);
    static_assert(gcl::mp::partial<std::is_same>::type<int, int>::value);

    namespace concatenate
    {
        template <typename... Ts>
        struct pack {};
        using T1 = pack<int, double>;
        using T2 = pack<char, float>;
        static_assert(std::is_same_v<concatenate_t<T1, T2>, pack<int, double, char, float>>);
    }
}
namespace gcl::mp::tests::pack_traits
{
    template <typename... Ts>
    struct pack_type {};

    using base_type = pack_type<int, char, float>;
    using pack_traits_type = gcl::mp::pack_traits<base_type>;

    static_assert(std::is_same_v<pack_traits_type::type, base_type>);
    static_assert(pack_traits_type::size == 3);
    static_assert(pack_traits_type::size == std::tuple_size_v<pack_traits_type::arguments>);
    static_assert(std::is_same_v<pack_traits_type::arguments, std::tuple<int, char, float>>);
    static_assert(std::is_same_v<base_type, pack_traits_type::unpack_as<pack_type>>);

    static_assert(pack_traits_type::is_instance_of_v<pack_type>);

    static_assert(std::is_same_v<pack_traits_type::type_at<1>, char>);
    static_assert(pack_traits_type::contains<char>);

    static_assert(pack_traits_type::satisfy_trait_v<std::is_standard_layout>);
    static_assert(not pack_traits_type::satisfy_trait_v<std::is_pointer>);
}
namespace gcl::mp::tests::filters
{
    template <typename... Ts>
    struct pack_type {};

    using pack_with_some_ptrs = pack_type<int, char*, char, int*>;
    // using only_pointers = gcl::mp::pack_traits<pack_with_some_ptrs>::filters<std::is_pointer>;
    // static_assert(std::is_same_v<pack_type<char*, int*>, only_pointers>)
}
