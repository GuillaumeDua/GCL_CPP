#pragma once

#include <type_traits>
namespace gcl::mp::type_traits
{
    template <class T>
    struct is_template : std::false_type {};
    template <class... T_args, template <class...> class T>
    struct is_template<T<T_args...>> : std::true_type {};
    template <auto... values, template <auto...> class T>
    struct is_template<T<values...>> : std::true_type {};
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
    constexpr static inline auto is_brace_constructible_v = is_brace_constructible<T, Args...>::value;

    template <bool evaluation>
    using if_t = std::conditional_t<evaluation, std::true_type, std::false_type>;
    template <bool evaluation>
    constexpr static inline auto if_v = std::conditional_t<evaluation, std::true_type, std::false_type>::value;
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
namespace gcl::mp::type_traits::tests::if_t
{
    static_assert(std::is_same_v<type_traits::if_t<true>, std::true_type>);
    static_assert(std::is_same_v<type_traits::if_t<false>, std::false_type>);
    static_assert(type_traits::if_v<true> == true);
    static_assert(type_traits::if_v<false> == false);
}
#if __cpp_concepts
#include <concepts>
namespace gcl::mp::type_traits::tests::if_t
{
    // clang-format off
    template <typename T>
    concept is_red_colored = requires(T)
    {
        { T::color == decltype(T::color)::red } -> std::convertible_to<bool>;
        { type_traits::if_t<T::color == decltype(T::color)::red>{}} -> std::same_as<std::true_type>;
    };
    enum colors { red, blue, green };
    struct smthg_blue { constexpr static auto color = colors::blue; };
    struct smthg_red { constexpr static auto color = colors::red; };
    // clang-format on

    static_assert(not is_red_colored<smthg_blue>);
    static_assert(is_red_colored<smthg_red>);
}
#endif
