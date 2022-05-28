#pragma once

#include <type_traits>
#include <utility>

namespace gcl::mp
{
    // is_pack<(ttp|ttps)> -> true if tttp<ttps...>
    template <typename...>
    struct is_pack {
        constexpr static bool value = false;
    };
    template <template <typename...> typename pack_type, typename... Ts>
    struct is_pack<pack_type<Ts...>> {
        constexpr static bool value = true;
    };
    template <typename... Ts>
    constexpr bool is_pack_v = is_pack<Ts...>::value;

    template <typename T>
    concept PackType = is_pack_v<T>;

    template <template <typename...> class T, typename... Ts>
    class pack_as {

        template <typename... types>
        struct impl {
            using type = T<types...>;
        };
        template <template <typename...> class tuple_type, typename... types>
        struct impl<tuple_type<types...>> {
            using type = T<types...>;
        };

      public:
        using type = typename impl<Ts...>::type;
    };
    template <template <typename...> class T, typename... Ts>
    using pack_as_t = typename pack_as<T, Ts...>::type;

    template <typename T, typename tuple_type>
    struct count_of;
    template <typename T, template <typename...> class tuple_type, typename... types>
    struct count_of<T, tuple_type<types...>> {
        constexpr static std::size_t value = ((std::size_t{std::is_same_v<T, types>} + ...));
    };
    template <typename T, typename tuple_type>
    constexpr bool count_of_v = count_of<T, tuple_type>::value;

    template <typename T, typename tuple_type>
    struct contains;
    template <typename T, template <typename...> class tuple_type, typename... types>
    struct contains<T, tuple_type<types...>> {
        constexpr static bool value = (std::is_same_v<T, types> or ...);
    };
    template <typename T, typename tuple_type>
    constexpr bool contains_v = contains<T, tuple_type>::value;

    template <typename tuple_type>
    struct unique;
    template <template <typename...> class tuple_type, typename first, typename... rest>
    struct unique<tuple_type<first, rest...>> {
        constexpr static bool value = (not(std::is_same_v<first, rest> or ...)) and unique<tuple_type<rest...>>::value;
    };
    template <template <typename...> class tuple_type, typename types>
    struct unique<tuple_type<types>> {
        constexpr static bool value = true;
    };
    template <typename T>
    constexpr bool unique_v = unique<T>::value;
    
    // wip : remove dependency to std::tuple, add index_of
    #ifdef false
    template <typename tuple_type>
    class deduplicate_types {
        static constexpr decltype(auto) deduplicate(tuple_type&& value) // noexcept(auto)
        {
            using tuple_type_t = std::remove_cvref_t<tuple_type>;
            return [&]<std::size_t... indexes>(std::index_sequence<indexes...>)
            {
                return std::tuple_cat([&]<std::size_t index>() {
                    using type_at_index = std::tuple_element_t<index, tuple_type_t>;
                    if constexpr (mp::index_of_v<type_at_index, tuple_type_t> == index)
                        return std::tuple<type_at_index>(std::get<index>(value));
                    else
                        return std::tuple<>{};
                }.template operator()<indexes>()...);
            }
            (std::make_index_sequence<std::tuple_size_v<tuple_type_t>>());
        }

      public:
        using type = decltype(deduplicate(std::declval<tuple_type>()));
    };
    template <typename tuple_type>
    using deduplicate_types_t = deduplicate_types<tuple_type>::type;
    #endif

    #if defined(_MSC_VER) and not defined(__clang__)
    # pragma message("gcl::mp::index_of : disabled on msvc-cl")
    #else
    template <typename T, typename tuple_type>
    struct index_of;
    template <typename T, template <typename...> class tuple_type, typename... types>
    struct index_of<T, tuple_type<types...>> {
        static_assert(
            contains_v<T, tuple_type<types...>>, "only one occurence allowed"); // duplicates parameter-pack unroll
        constexpr static std::size_t value =
            []<std::size_t... indexes>(std::index_sequence<indexes...>) constexpr noexcept
        {
            return ((std::is_same_v<T, types> ? indexes : 0) + ...);
        }
        (std::make_index_sequence<sizeof...(types)>{});
    };
    template <typename T, typename tuple_type>
    constexpr std::size_t index_of_v = index_of<T, tuple_type>::value;
    #endif

    template <std::size_t index, class tuple_type>
    struct type_at;
    template <std::size_t index, template <typename...> class tuple_type, class first, class... rest>
    struct type_at<index, tuple_type<first, rest...>> : type_at<(index - 1), tuple_type<rest...>> {};
    template <class first, template <typename...> class tuple_type, class... rest>
    struct type_at<0, tuple_type<first, rest...>> {
        using type = first;        
    };
    template <std::size_t index, template <typename...> class tuple_type>
    struct type_at<index, tuple_type<>> {
        static_assert([]() constexpr { return false; }(), "out of range");
    };
    template <std::size_t index, class tuple_type>
    using type_at_t = typename type_at<index, tuple_type>::type;

    template <typename tuple_type>
    struct size;
    template <template <typename...> class tuple_type, typename... types>
    struct size<tuple_type<types...>> {
        constexpr static std::size_t value = sizeof...(types);
    };
    template <typename tuple_type>
    constexpr std::size_t size_v = size<tuple_type>::value;

    template <typename tuple_type>
    struct empty;
    template <template <typename...> class tuple_type, typename... types>
    struct empty<tuple_type<types...>> {
        constexpr static bool value = (sizeof...(types) == 0);
    };
    template <typename tuple_type>
    constexpr bool empty_v = empty<tuple_type>::value;

    template <typename tuple_type>
    struct first;
    template <template <typename...> class tuple_type, typename first_element_t, typename... rest>
    struct first<tuple_type<first_element_t, rest...>> {
        using type = first_element_t;
    };
    template <typename tuple_type>
    using first_t = typename first<tuple_type>::type;
}

#if false
// GCC only
namespace gcl::mp::experimental
{   // see WIP https://godbolt.org/z/qoo5KsTY1

    template <typename T>
    struct is_template : std::false_type {};
    template <template <typename...> typename T, typename... Ts>
    struct is_template<T<Ts...>> : std::true_type {};

    template <typename tuple_type, typename functor_type>
    requires(is_template<tuple_type>::value) constexpr functor_type for_each_index(functor_type&& func_value)
    {
        [&]<template <typename...> class pack, typename... types>(pack<types...>) constexpr
        {
            [&]<std::size_t... indexes>(std::index_sequence<indexes...>) constexpr
            {
                (func_value.template operator()<types, indexes>(), ...);
            }
            (std::make_index_sequence<sizeof...(types)>{});
        }
        (tuple_type{});
        return std::move(func_value);
    };

    template <typename tuple_type, typename functor_type>
    requires(is_template<tuple_type>::value) constexpr functor_type for_each(functor_type&& func_value)
    {
        [&]<template <typename...> class pack, typename... types>(pack<types...>)
        {
            ((func_value.template operator()<types>()), ...);
        }
        (tuple_type{});
        return std::move(func_value);
    };
}
#endif

namespace gcl::mp::meta::tests::pack_traits
{
    template <typename ... Ts>
    struct pack_type {};

    using pack_t = pack_type<int, char>;

    namespace contains
    {
        static_assert(gcl::mp::contains_v<int, pack_t>);
        static_assert(gcl::mp::contains_v<char, pack_t>);
        static_assert(not gcl::mp::contains_v<double, pack_t>);
    }
    namespace unique {
        static_assert(gcl::mp::unique_v<pack_type<int, double, char>>);
        static_assert(not gcl::mp::unique_v<pack_type<int, double, int, char>>);
        static_assert(not gcl::mp::unique_v<pack_type<bool, int, double, int, char>>);
    }
    #ifdef false
    namespace deduplicate
    {
        using type = std::tuple<char, char, float, float, char, bool, int, int, int, bool>;
        using expected = std::tuple<char, float, bool, int>;

        static_assert(std::is_same_v<decltype(tuple_utils::deduplicate(std::declval<deduplicated_type>())), expected>);
    }
    #endif
    namespace type_at
    {
        static_assert(std::is_same_v<gcl::mp::type_at_t<0, pack_t>, int>);
        static_assert(std::is_same_v<gcl::mp::type_at_t<1, pack_t>, char>);
    }
    #if defined(_MSC_VER) and not defined(__clang__)
    #else
    namespace index_of
    {
        static_assert(gcl::mp::index_of_v<int, pack_t> == 0);
        static_assert(gcl::mp::index_of_v<char, pack_t> == 1);
        static_assert(std::is_same_v<gcl::mp::type_at_t<gcl::mp::index_of_v<int, pack_t>, pack_t>, int>);
    }
    #endif
    namespace size
    {
        static_assert(gcl::mp::size_v<pack_t> == 2);
        static_assert(gcl::mp::size_v<pack_type<>> == 0);
    }
    namespace first
    {
        static_assert(std::is_same_v<int, gcl::mp::first_t<pack_t>>);
    }
    namespace pack_as
    {
        template <typename ...>
        struct other_pack_type {};
        static_assert(
            std::is_same_v<other_pack_type<int, char>, gcl::mp::pack_as_t<other_pack_type, int, char>>); // types
        static_assert(std::is_same_v<other_pack_type<int, char>, gcl::mp::pack_as_t<other_pack_type, pack_t>>); // pack
        static_assert(std::is_same_v<pack_type<int, char>, gcl::mp::pack_as_t<pack_type, pack_t>>); // repack into the same type
    }
}