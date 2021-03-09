#pragma once

#include <gcl/functional.hpp>

#include <tuple>
#include <type_traits>

namespace gcl::mp
{
    template <typename Function>
    class function_traits {
        struct informations {
            const bool is_const_v;
            const bool is_member_function_v;
            const bool is_no_except;
            // is constexpr
        };

        template <typename>
        struct member_function_traits_impl;
        template <typename ClassType, typename ReturnType, typename... Arguments>
        struct member_function_traits_impl<ReturnType (ClassType::*)(Arguments...)> {
            using result_type = ReturnType;
            using class_type = ClassType;
            using arguments = std::tuple<Arguments...>;
            template <std::size_t N>
            using argument = typename std::tuple_element_t<N, std::tuple<Arguments...>>;
        };
        template <typename ClassType, typename ReturnType, typename... Arguments>
        struct member_function_traits_impl<ReturnType (ClassType::*)(Arguments...) noexcept>
            : member_function_traits_impl<ReturnType (ClassType::*)(Arguments...)> {};
        template <typename ClassType, typename ReturnType, typename... Arguments>
        struct member_function_traits_impl<ReturnType (ClassType::*)(Arguments...) const>
            : member_function_traits_impl<ReturnType (ClassType::*)(Arguments...)> {};
        template <typename ClassType, typename ReturnType, typename... Arguments>
        struct member_function_traits_impl<ReturnType (ClassType::*)(Arguments...) const noexcept>
            : member_function_traits_impl<ReturnType (ClassType::*)(Arguments...)> {};

        template <typename>
        struct function_traits_impl;
        template <typename ReturnType, typename... Arguments>
        struct function_traits_impl<ReturnType (*)(Arguments...)> {
            using result_type = ReturnType;
            using arguments = std::tuple<Arguments...>;
            template <std::size_t N>
            using argument = typename std::tuple_element_t<N, std::tuple<Arguments...>>;
        };
        template <typename ReturnType, typename... Arguments>
        struct function_traits_impl<ReturnType (*)(Arguments...) noexcept>
            : function_traits_impl<ReturnType (*)(Arguments...)> {};

      public:
        using type = std::conditional_t<
            std::is_member_function_pointer_v<Function>,
            member_function_traits_impl<Function>,
            function_traits_impl<Function>>;
    };

    template <typename Function>
    using function_traits_t = typename function_traits<Function>::type;
    // using function_traits = member_function_traits_impl<decltype(&Function::operator())>;
}

namespace gcl::mp::test
{
    struct toto
    {
        int const_member(char) const { return 42; }
        int not_const_member(char) { return 42; }
        int noexcept_const_member(char) const noexcept { return 42; }
        int noexcept_not_const_member(char) noexcept { return 42; }
    };
    static_assert(std::is_same_v<int, function_traits_t<decltype(&toto::const_member)>::result_type>);
    static_assert(std::is_same_v<int, function_traits_t<decltype(&toto::not_const_member)>::result_type>);
    static_assert(std::is_same_v<int, function_traits_t<decltype(&toto::noexcept_const_member)>::result_type>);
    static_assert(std::is_same_v<int, function_traits_t<decltype(&toto::noexcept_not_const_member)>::result_type>);

    int some_function(char) { return 42; };
    int noexcept_function(char) noexcept { return 42; };
    static_assert(std::is_same_v<char, function_traits_t<decltype(&some_function)>::argument<0>>);
    static_assert(std::is_same_v<char, function_traits_t<decltype(&noexcept_function)>::argument<0>>);
}