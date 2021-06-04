#pragma once

#include <type_traits>
#include <utility>

namespace gcl::mp
{
    template <typename T, typename tuple_type>
    struct contains;
    template <typename T, template <typename...> class tuple_type, typename... types>
    struct contains<T, tuple_type<types...>> {
        constexpr static bool value = ((std::size_t{std::is_same_v<T, types>} + ...)) == 1;
    };
    template <typename T, typename tuple_type>
    constexpr bool contains_v = contains<T, tuple_type>::value;

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
}

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
    namespace index_of
    {
        static_assert(gcl::mp::index_of_v<int, pack_t> == 0);
        static_assert(gcl::mp::index_of_v<char, pack_t> == 1);
    }
    namespace type_at
    {
        static_assert(std::is_same_v<gcl::mp::type_at_t<0, pack_t>, int>);
        static_assert(std::is_same_v<gcl::mp::type_at_t<1, pack_t>, char>);
        static_assert(std::is_same_v<gcl::mp::type_at_t<
            gcl::mp::index_of_v<int, pack_t>, pack_t>,
            int>);
    }
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