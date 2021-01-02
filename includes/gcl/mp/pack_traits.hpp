#pragma once

#include <gcl/mp/type_traits.hpp>

namespace gcl::mp
{
    template <typename T, typename = std::enable_if_t<gcl::mp::type_traits::is_template_v<T>>>
    struct pack_traits {
        using type = T;
        using arguments = type_traits::pack_arguments_t<T>;
        template <template <class...> class template_type>
        using unpack_as = type_traits::pack_arguments_t<T, template_type>;

        static constexpr inline auto size = std::tuple_size_v<arguments>;
        template <template <class...> class template_type>
        static inline constexpr auto is_instance_of_v = type_traits::is_instance_of_v<T, template_type>;
    };
}

namespace gcl::mp::test
{
    using base_type = gcl::mp::type_traits::tests::pack<int, char, float>;
    using pack_traits_type = pack_traits<base_type>;

    static_assert(std::is_same_v<pack_traits_type::type, base_type>);
    static_assert(pack_traits_type::size == 3);
    static_assert(pack_traits_type::size == std::tuple_size_v<pack_traits_type::arguments>);
    static_assert(std::is_same_v<pack_traits_type::arguments, std::tuple<int, char, float>>);
    static_assert(std::is_same_v<base_type, pack_traits_type::unpack_as<gcl::mp::type_traits::tests::pack>>);
}