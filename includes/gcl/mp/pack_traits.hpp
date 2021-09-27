#pragma once

#include <gcl/mp/type_traits.hpp>
#include <gcl/mp/value_traits.hpp>
#include <gcl/mp/utility.hpp>
#include <tuple>
#include <array>
#include <algorithm>

namespace gcl::mp::type_traits
{
    // is_pack
    template <typename T>
    struct is_pack {
        constexpr static bool value = false;
    };
    template <template <typename...> typename pack_type, typename ... Ts>
    struct is_pack<pack_type<Ts...>> {
        constexpr static bool value = true;
    };
    template <typename T>
    constexpr bool is_pack_v = is_pack<T>::value;

    // repack_into
    template <template <typename...> typename destination, typename from>
    struct repack_into {
        static_assert([](){ return false; }(), "invalid usage");
    };
    template <template <typename...> typename destination, template <typename...> typename from, typename ... Ts>
    struct repack_into<destination, from<Ts...>> {
        using type = destination<Ts...>;
    };
    template <template <typename...> typename destination, typename from>
    using repack_into_t = typename repack_into<destination, from>::type;

    // pack_arguments_as<tttp, (ttps...|tttp)>
    template <template <typename...> class T, typename... Ts>
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
    template <template <typename...> class T, typename... Ts>
    using pack_arguments_as_t = typename pack_arguments_as<T, Ts...>::type;
    template <typename... Ts>
    using arguments_index_sequence_t =
        decltype(std::make_index_sequence<std::tuple_size_v<pack_arguments_as_t<std::tuple, Ts...>>>{});

    // concate / merge
    template <typename T, typename U>
    struct concatenate;
    template <template <typename...> typename T, typename... Ts, typename... Us>
    struct concatenate<T<Ts...>, T<Us...>> {
        using type = T<Ts..., Us...>;
    };
    template <typename T, typename U>
    using concatenate_t = typename concatenate<T, U>::type;

    // type_at
    template <std::size_t N, typename... Ts>
    using type_at = typename std::tuple_element<N, std::tuple<Ts...>>;
    template <std::size_t N, typename... Ts>
    using type_at_t = typename type_at<N, Ts...>::type;

    // count_of
    template <typename T, typename... Ts>
    struct count_of {
        constexpr inline static auto value = []<typename ... Us>(std::tuple<Us...>) constexpr
        {
            std::size_t index{0};
            ((std::is_same_v<T, Us> ? ++index : 0), ...);
            return index;
        }
        (gcl::mp::type_traits::pack_arguments_as_t<std::tuple, Ts...>{});
    };
    template <typename T, typename... Ts>
    constexpr inline auto count_of_v = count_of<T, Ts...>::value;

    // todo :
    //  unique : universal template parameter, to merge ttps/nttps

    // are unique_(n)ttps
    template <typename... Ts>
    struct are_unique_ttps {
        constexpr static bool value = []<typename first, typename... rest>()
        {
            return (not(std::is_same_v<first, rest> or ...)) and are_unique_ttps<rest...>::value;
        }
        .template operator()<Ts...>();
    };
    template <>
    struct are_unique_ttps<> {
        constexpr static bool value = true;
    };
    template <typename... Ts>
    constexpr auto are_unique_ttps_v = are_unique_ttps<Ts...>::value;

    template <auto... values>
    struct are_unique_nttps {
        constexpr static bool value = []<auto first, auto... rest>()
        {
            return (not((first == rest) or ...)) and are_unique_nttps<rest...>::value;
        }
        .template operator()<values...>();
    };
    template <>
    struct are_unique_nttps<> {
        constexpr static bool value = true;
    };
    template <auto... values>
    constexpr auto are_unique_nttps_v = are_unique_nttps<values...>::value;

    // unique
    template <typename T>
    struct unique_ttps {
        static_assert([]() { return false; }(), "unique_ttps : not a template-template parameter");
    };
    template <template <typename...> typename T, typename... Ts>
    struct unique_ttps<T<Ts...>> {
        constexpr static bool value = are_unique_ttps_v<Ts...>;
    };
    template <typename T>
    constexpr bool has_unique_ttps_v = unique_ttps<T>::value;

    template <typename T>
    struct unique_nttps {
        static_assert([]() { return false; }(), "unique_ttps : not a template-template parameter");
    };
    template <template <auto...> typename T, auto... values>
    struct unique_nttps<T<values...>> {
        constexpr static bool value = are_unique_nttps_v<values...>;
    };
    template <typename T>
    constexpr bool has_unique_nttps_v = unique_nttps<T>::value;

    // transform
    template <typename T, template <typename> typename transformation>
    struct transform;
    template <template <typename...> typename TypePack, template <typename> typename transformation, typename... Ts>
    struct transform<TypePack<Ts...>, transformation> {
        using type = TypePack<transformation<Ts>...>;
    };
    template <typename T, template <typename> typename transformation>
    using transform_t = typename transform<T, transformation>::type;

    // trait_as_mask / filter
    template <template <typename...> class T, typename... Ts>
    struct trait_as_mask {

        constexpr static auto value = []<typename... PackArgs>(std::tuple<PackArgs...>) consteval
        {
            using TupleType = std::tuple<PackArgs...>;
            return []<std::size_t... I>(std::index_sequence<I...>) consteval
            {
                return std::array<bool, sizeof...(I)>{T<std::tuple_element_t<I, TupleType>>::value...};
            }
            (std::make_index_sequence<sizeof...(PackArgs)>{});
        }
        (pack_arguments_as_t<std::tuple, Ts...>{});
    };
    template <template <typename...> class T, typename... Ts>
    constexpr auto trait_as_mask_v = trait_as_mask<T, Ts...>::value;

    // reverse
    template <class T>
    class reverse {
        static_assert(gcl::mp::type_traits::is_template_v<T>);

        template <template <typename...> class Type, typename... Ts>
        constexpr static auto impl(Type<Ts...>)
        {
            // todo : consteval (works with GCC 10.2 and MSVC, not clang 11.0.1)
            return []<std::size_t... indexes>(std::index_sequence<indexes...>) constexpr
            {
                using Ts_as_tuple = std::tuple<Ts...>;
                using return_type = Type<std::tuple_element_t<indexes, Ts_as_tuple>...>;
                return return_type{};
            }
            (gcl::mp::utility::make_reverse_index_sequence<sizeof...(Ts)>{});
        }
        template <template <auto...> class Type, auto... values>
        constexpr static auto impl(Type<values...>)
        {
            return []<std::size_t... indexes>(std::index_sequence<indexes...>) constexpr
            {
                constexpr auto Ts_as_tuple = std::tuple{values...};
                using return_type = Type<std::get<indexes>(Ts_as_tuple)...>;
                return return_type{};
            }
            (gcl::mp::utility::make_reverse_index_sequence<sizeof...(values)>{});
        }

      public:
        using type = decltype(impl(std::declval<T>()));
    };
    template <class T>
    using reverse_t = typename reverse<T>::type;

    // rindex_of
    template <typename tuple_type, typename T>
    struct rindex_of;
    template <template <typename...> typename tuple_type, typename T>
    struct rindex_of<tuple_type<>, T> {
        static_assert([]() { return false; }(), "(r)index_of : not found");
    };
    template <template <typename...> typename tuple_type, typename... rest, typename T>
    struct rindex_of<tuple_type<T, rest...>, T> {
        constexpr static std::size_t value = sizeof...(rest);
    };
    template <template <typename...> typename tuple_type, typename first, typename... rest, typename T>
    struct rindex_of<tuple_type<first, rest...>, T> {
        constexpr static std::size_t value = rindex_of<tuple_type<rest...>, T>::value;
    };
    template <typename tuple_type, typename T>
    constexpr std::size_t rindex_of_v = rindex_of<tuple_type, T>::value;

    // index_of
    template <typename tuple_type, typename T>
    struct index_of {
        constexpr static auto value =
            std::tuple_size_v<std::remove_cvref_t<tuple_type>> - rindex_of<tuple_type, T>::value - 1;
    };
    template <typename tuple_type, typename T>
    constexpr std::size_t index_of_v = index_of<tuple_type, T>::value;

    template <typename T, typename... Ts>
    using contains = std::disjunction<std::is_same<T, Ts>...>;
    template <typename T, typename... Ts>
    constexpr inline auto contains_v = contains<T, Ts...>::value;

    template <typename T, template <typename> typename trait>
    struct filters;
    template <template <typename...> typename T, template <typename> typename trait, typename... Ts>
    struct filters<T<Ts...>, trait> {
        template <typename Type>
        using impl = std::conditional_t<trait<Type>::value, std::tuple<Type>, std::tuple<>>;

      public:
        using type = pack_arguments_as_t<T, decltype(std::tuple_cat(impl<Ts>{}...))>;

        // todo : index_sequence -> { true, false, true } -> { 1, 3 }
        // WIP : https://godbolt.org/z/PrxdWf
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
        using reverse_type = typename gcl::mp::type_traits::reverse_t<type>;
        template <template <class...> class template_type = std::tuple>
        using arguments_as = type_traits::template pack_arguments_as_t<template_type, type>;
        template <size_t N>
        using type_at = type_traits::type_at_t<N, Ts...>; // typename std::tuple_element<N, arguments>::type;

        template <typename U>
        static constexpr inline auto index_of_v = gcl::mp::type_traits::index_of_v<U, arguments_as<>>;
        template <typename U>
        static constexpr inline auto rindex_of_v = gcl::mp::type_traits::rindex_of_v<U, arguments_as<>>;

        static constexpr inline auto size = std::tuple_size_v<arguments_as<std::tuple>>;
        template <typename U>
        static constexpr inline /*auto*/ bool contains = type_traits::contains_v<U, Ts...>;
        template <template <class...> class template_type>
        static inline constexpr /*auto*/ bool is_instance_of_v = type_traits::is_instance_of_v<type, template_type>;

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
}

#if defined(GCL_ENABLE_COMPILE_TIME_TESTS)
namespace gcl::mp::type_traits::tests
{
    static_assert(std::is_same_v<gcl::mp::type_traits::type_at_t<2, char, bool, int, float>, int>);
    static_assert(gcl::mp::type_traits::contains_v<int, char, bool, int, float>);

    static_assert(gcl::mp::type_traits::count_of_v<int, char, double, int, char> == 1);
    static_assert(gcl::mp::type_traits::count_of_v<int, std::tuple<char, double, int, char>> == 1);

    namespace transform
    {
        using type_container = std::tuple<int &&, char &>;
        using expected_type = std::tuple<int, char>;
        static_assert(std::is_same_v<
            expected_type,
            gcl::mp::type_traits::transform_t<type_container, std::remove_reference_t>>
        );
    }
    namespace pack_arguments
    {
        template <typename... Ts>
        struct pack_type {};

        using toto = typename gcl::mp::type_traits::pack_arguments_as_t<std::tuple, pack_type<int, double, float>>;
        static_assert(std::is_same_v<toto, std::tuple<int, double, float>>);
        static_assert(
            std::is_same_v<pack_type<int, double, float>, gcl::mp::type_traits::pack_arguments_as_t<pack_type, toto>>);
    }
    namespace arguments_index_sequence
    {
        using args_index_sequence_result_t = gcl::mp::type_traits::arguments_index_sequence_t<int, double, float>;
        using expected_result_t = decltype(std::make_index_sequence<3>());
        static_assert(std::is_same_v<args_index_sequence_result_t, expected_result_t>);

        using args_as_pack_sequence_result_t = gcl::mp::type_traits::arguments_index_sequence_t<std::tuple<int, double, float>>;
        static_assert(std::is_same_v<args_as_pack_sequence_result_t, expected_result_t>);
    }
    namespace trait_as_mask
    {
        template <typename T>
        using is_int = std::is_same<int, T>;

        constexpr auto result = gcl::mp::type_traits::trait_as_mask_v<is_int, bool, int, char>;
        constexpr auto expected_result = std::array{false, true, false};
        static_assert(result == expected_result);

        constexpr auto result_of_pack = gcl::mp::type_traits::trait_as_mask_v<is_int, std::tuple<bool, int, char>>;
        static_assert(result_of_pack == expected_result);
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
        // average case
        static_assert(gcl::mp::type_traits::index_of_v<char, type_pack> == 2);
        static_assert(gcl::mp::type_traits::index_of_v<char, 
            int, double, char, float> == 2);
        // corner cases
        static_assert(gcl::mp::type_traits::index_of_v<int, type_pack> == 0);
        static_assert(gcl::mp::type_traits::index_of_v<int, int, double, char, float> == 0);
        static_assert(gcl::mp::type_traits::index_of_v<float, type_pack> == 3);
        static_assert(gcl::mp::type_traits::index_of_v<float, int, double, char, float> == 3);
    }
    namespace are_unique
    {
        static_assert(are_unique_ttps_v<int, double, char>);
        static_assert(not are_unique_ttps_v<int, double, int, char>);
        static_assert(not are_unique_ttps_v<bool, int, double, int, char>);
    }
    namespace reverse
    {
        namespace parameter_pack_of_type
        {
            using type = std::tuple<int, double, float, char>;
            using reversed_type = gcl::mp::type_traits::reverse_t<type>;

            static_assert(std::is_same_v<type, gcl::mp::type_traits::reverse_t<gcl::mp::type_traits::reverse_t<type>>>);
            static_assert(std::is_same_v<reversed_type, std::tuple<char, float, double, int>>);
        }
        namespace parameter_pack_of_values
        {
            template <auto... values>
            struct values_container {};
            using type = values_container<123, 'a', 42U, 55LL>;
            using reversed_type = gcl::mp::type_traits::reverse_t<type>;

            static_assert(std::is_same_v<type, gcl::mp::type_traits::reverse_t<gcl::mp::type_traits::reverse_t<type>>>);
            static_assert(std::is_same_v<reversed_type, values_container<55LL, 42U, 'a', 123>>);
        }
    }
}
namespace gcl::mp::tests::pack_traits
{
    template <typename... Ts>
    struct pack_type {};

    using base_type = pack_type<int, char, float>;
    using pack_traits_type = gcl::mp::pack_traits<base_type>;

    static_assert(std::is_same_v<pack_traits_type::type, base_type>);
    static_assert(std::is_same_v<pack_traits_type::reverse_type, pack_type<float, char, int>>);
    static_assert(std::is_same_v<pack_traits_type::type, gcl::mp::type_traits::reverse_t<pack_traits_type::reverse_type>>);

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

    // Clang error :
    //  - error: invalid operands to binary expression ('const auto' and 'int')
    //  see https://stackoverflow.com/questions/67127710/clang-invalid-operands-to-binary-expression-const-auto-and-int-with-cons
    //  see https://stackoverflow.com/questions/67123073/clang-constexpr-function-evaluation
    #if not __clang__
    static_assert(pack_type_with_repetitions_trait::index_of_v<char> == 1);
    static_assert(pack_type_with_repetitions_trait::first_index_of_v<char> == 1);
    static_assert(pack_type_with_repetitions_trait::last_index_of_v<char> == 4);
    #endif

    namespace filters
    {
        using T1 = pack_type<int, int*, char, char*, float>;
        using T1_pack_trait = gcl::mp::pack_traits<T1>;
        static_assert(std::is_same_v<pack_type<int*, char*>, T1_pack_trait::filters<std::is_pointer>>);
    }
}
#endif