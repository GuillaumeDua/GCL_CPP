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