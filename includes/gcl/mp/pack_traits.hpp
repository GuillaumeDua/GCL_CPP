#pragma once

#include <gcl/mp/type_traits.hpp>
#include <tuple>
#include <array>
#include <algorithm>

namespace gcl::mp::type_traits
{
    template <template <typename ...> class T, typename ... Ts>
    class pack_arguments_as {
        template <template <typename...> class PackType, typename... PackArgs>
        constexpr static auto impl(PackType<PackArgs...>)
        {
            return T<PackArgs...>{};
        }
        template <typename... PackArgs>
        constexpr static auto impl(PackArgs...)
        {
            return T<PackArgs...>{};
        }
      public:
        using type = decltype(impl(std::declval<Ts>()...));
    };
    template <template <typename ...> class T, typename ... Ts>
    using pack_arguments_as_t = typename pack_arguments_as<T, Ts...>::type;

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

    template <typename to_find, typename pack_type>
    class index_of {
        template <auto distance_algorithm, template <typename...> class PackType, typename... PackArgs>
        consteval static auto impl(PackType<PackArgs...>)
        {
            constexpr auto index =
                []<typename TupleType, std::size_t... I>(TupleType, std::index_sequence<I...>) consteval
            {
                constexpr std::array<bool, sizeof...(I)> matches = {
                    std::is_same_v<to_find, std::tuple_element_t<I, TupleType>>...};
                static_assert(std::size(matches) == sizeof...(I));
                using matches_iterator_type = decltype(std::cbegin(matches));
                return distance_algorithm(matches);
            }
            (std::tuple<PackArgs...>{}, std::make_index_sequence<sizeof...(PackArgs)>{});
            static_assert(index != sizeof...(PackArgs), "index_of : no match");
            return index;
        }

        constexpr static auto from_begin = []<class ContainerType>(ContainerType container) consteval
        {
            return std::distance(std::cbegin(container), std::find(std::cbegin(container), std::cend(container), true));
        };
        constexpr static auto from_end = []<class ContainerType>(ContainerType container) consteval
        {
            return std::size(container) - 1 -
                   std::distance(
                       std::crbegin(container), std::find(std::crbegin(container), std::crend(container), true));
        };

      public:
        constexpr static auto value = impl<from_begin>(pack_type{});
        constexpr static auto first_value = value;
        constexpr static auto last_value = impl<from_end>(pack_type{});
    };

    template <typename to_find, typename pack_type>
    constexpr static auto index_of_v = index_of<to_find, pack_type>::value;
    template <typename to_find, typename pack_type>
    constexpr static auto first_index_of_v = index_of<to_find, pack_type>::first_value;
    template <typename to_find, typename pack_type>
    constexpr static auto last_index_of_v = index_of<to_find, pack_type>::last_value;

    template <typename T, typename... Ts>
    using contains = std::disjunction<std::is_same<T, Ts>...>;
    template <typename T, typename... Ts>
    static constexpr inline auto contains_v = contains<T, Ts...>::value;

    template <typename T, template <typename> typename trait>
    struct filters;
    template <template <typename...> typename T, template <typename> typename trait, typename... Ts>
    struct filters<T<Ts...>, trait> {
        template <typename Type>
        using impl = std::conditional_t<trait<Type>::value, std::tuple<Type>, std::tuple<>>;

      public:
        using type = pack_arguments_as_t<T, decltype(std::tuple_cat(impl<Ts>{}...))>;
    };
    template <typename T, template <typename> typename trait>
    using filters_t = typename filters<T, trait>::type;
}
namespace gcl::mp
{
    template <typename T, typename = std::enable_if_t<gcl::mp::type_traits::is_template_v<T>>>
    struct pack_traits;
    template <template <typename...> typename T, typename... Ts>
    struct pack_traits<T<Ts...>> {
        using type = T<Ts...>;
        template <template <class...> class template_type = std::tuple>
        using arguments_as = type_traits::pack_arguments_as_t<template_type, type>;
        template <size_t N>
        using type_at = type_traits::type_at_t<N, Ts...>; // typename std::tuple_element<N, arguments>::type;

        template <typename U>
        static constexpr inline auto index_of_v = gcl::mp::type_traits::index_of_v<U, arguments_as<>>;
        template <typename U>
        static constexpr inline auto first_index_of_v = gcl::mp::type_traits::first_index_of_v<U, arguments_as<>>;
        template <typename U>
        static constexpr inline auto last_index_of_v = gcl::mp::type_traits::last_index_of_v<U, arguments_as<>>;

        static constexpr inline auto size = std::tuple_size_v<arguments_as<std::tuple>>;
        template <typename U>
        static constexpr inline auto contains = type_traits::contains_v<U, Ts...>;
        template <template <class...> class template_type>
        static inline constexpr auto is_instance_of_v = type_traits::is_instance_of_v<type, template_type>;

        template <template <typename> typename trait>
        using satisfy_trait_t = std::conjunction<trait<Ts>...>;
        template <template <typename> typename trait>
        constexpr static inline bool satisfy_trait_v = satisfy_trait_t<trait>::value; //(trait<Ts>::value && ...)

        template <template <typename> typename trait>
        using filters = type_traits::filters_t<T<Ts...>, trait>;
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

namespace gcl::mp::type_traits::tests
{
    static_assert(std::is_same_v<gcl::mp::type_traits::type_at_t<2, char, bool, int, float>, int>);
    static_assert(gcl::mp::type_traits::contains_v<int, char, bool, int, float>);

    static_assert(gcl::mp::partial<std::is_same, int>::type<int>::value);
    static_assert(gcl::mp::partial<std::is_same>::type<int, int>::value);

    namespace pack_arguments
    {
        template <typename... Ts>
        struct pack_type {};

        using toto = typename gcl::mp::type_traits::pack_arguments_as_t<std::tuple, pack_type<int, double, float>>;
        static_assert(std::is_same_v<toto, std::tuple<int, double, float>>);
        static_assert(
            std::is_same_v<pack_type<int, double, float>, gcl::mp::type_traits::pack_arguments_as_t<pack_type, toto>>);
    }
    namespace concatenate
    {
        template <typename... Ts>
        struct pack {};
        using T1 = pack<int, double>;
        using T2 = pack<char, float>;
        static_assert(std::is_same_v<concatenate_t<T1, T2>, pack<int, double, char, float>>);
    }
    namespace filters
    {
        template <typename... Ts>
        struct pack {};
        using T1 = pack<int, int*,char, char*, float>;
        static_assert(std::is_same_v<filters_t<T1, std::is_pointer>, pack<int*, char*>>);
    }
    namespace index_of
    {
        template <typename... Ts>
        struct pack {};

        using type_pack = pack<int, double, char, float>;
        static_assert(gcl::mp::type_traits::index_of_v<char, type_pack> == 2);
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
    static_assert(pack_traits_type::size == std::tuple_size_v<pack_traits_type::arguments_as<>>);
    static_assert(std::is_same_v<pack_traits_type::arguments_as<>, std::tuple<int, char, float>>);
    static_assert(std::is_same_v<base_type, pack_traits_type::arguments_as<pack_type>>);

    static_assert(pack_traits_type::is_instance_of_v<pack_type>);

    static_assert(std::is_same_v<pack_traits_type::type_at<1>, char>);
    static_assert(pack_traits_type::contains<char>);

    static_assert(pack_traits_type::satisfy_trait_v<std::is_standard_layout>);
    static_assert(not pack_traits_type::satisfy_trait_v<std::is_pointer>);

    using pack_type_with_repetitions = pack_type<int, char, double, int, char>;
    using pack_type_with_repetitions_trait = gcl::mp::pack_traits<pack_type_with_repetitions>;

    static_assert(pack_type_with_repetitions_trait::index_of_v<char> == 1);
    static_assert(pack_type_with_repetitions_trait::first_index_of_v<char> == 1);
    static_assert(pack_type_with_repetitions_trait::last_index_of_v<char> == 4);

    namespace filters
    {
        using T1 = pack_type<int, int*, char, char*, float>;
        using T1_pack_trait = gcl::mp::pack_traits<T1>;
        static_assert(std::is_same_v<pack_type<int*, char*>, T1_pack_trait::filters<std::is_pointer>>);
    }
}
