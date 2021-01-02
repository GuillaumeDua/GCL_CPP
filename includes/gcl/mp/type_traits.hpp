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
}
