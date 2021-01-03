#pragma once

#include <gcl/mp/type_traits.hpp>

namespace gcl::mp::type_traits
{
    template <std::size_t N, typename... Ts>
    using type_at = typename std::tuple_element<N, std::tuple<Ts...>>::type;
    template <typename T, typename... Ts>
    using contains = std::disjunction<std::is_same<T, Ts>...>;
    template <typename T, typename... Ts>
    static constexpr inline auto contains_v = contains<T, Ts...>::value;
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
        using type_at = typename std::tuple_element<N, arguments>::type;

        static constexpr inline auto size = std::tuple_size_v<arguments>;
        template <typename U>
        static constexpr inline auto contains = type_traits::contains_v<U, Ts...>;
        template <template <class...> class template_type>
        static inline constexpr auto is_instance_of_v = type_traits::is_instance_of_v<type, template_type>;
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
}
namespace gcl::mp::tests::partial
{
    static_assert(gcl::mp::partial<std::is_same, int>::type<int>::value);
    static_assert(gcl::mp::partial<std::is_same>::type<int, int>::value);
}