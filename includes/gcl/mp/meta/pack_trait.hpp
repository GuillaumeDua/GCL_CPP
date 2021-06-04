#pragma once

#include <type_traits>
#include <utility>

namespace gcl::mp::meta
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
}

namespace gcl::mp::tests
{
    template <typename ... Ts>
    struct pack_type {};

    using pack_t = pack_type<int, char>;

    namespace contains
    {
        static_assert(gcl::mp::meta::contains_v<int, pack_t>);
        static_assert(gcl::mp::meta::contains_v<char, pack_t>);
        static_assert(not gcl::mp::meta::contains_v<double, pack_t>);
    }
    namespace index_of
    {
        static_assert(gcl::mp::meta::index_of_v<int, pack_t> == 0);
        static_assert(gcl::mp::meta::index_of_v<char, pack_t> == 1);
    }
    namespace type_at
    {
        static_assert(std::is_same_v<gcl::mp::meta::type_at_t<0, pack_t>, int>);
        static_assert(std::is_same_v<gcl::mp::meta::type_at_t<1, pack_t>, char>);
        static_assert(std::is_same_v<gcl::mp::meta::type_at_t<
            gcl::mp::meta::index_of_v<int, pack_t>, pack_t>,
            int>);
    }
}