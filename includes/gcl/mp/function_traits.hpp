#pragma once

// #include <gcl/functional.hpp>
#include <gcl/mp/type_tag.hpp>

#include <tuple>
#include <type_traits>

namespace gcl::mp
{
    struct tags {
        struct is_const {};
        struct is_volatile {};
        struct is_no_except {};

        /*struct is_lvalue_reference {};
        struct is_rvalue_reference {};
        struct is_constexpr {};
        struct is_lambda {};*/
    };

    template <typename Function>
    class function_traits {
#ifdef _MSC_VER
#pragma warning(disable : 4348)
// Clang, GCC does not consider such code as default template-parameter redefinition
#endif
        template <typename, class function_attr = type_tag::container<>>
        struct member_function_traits_impl;
        template <typename ClassType, typename ReturnType, typename... Arguments, class function_attr>
        struct member_function_traits_impl<ReturnType (ClassType::*)(Arguments...), function_attr> {
            using result_type = ReturnType;
            using class_type = ClassType;
            using arguments = std::tuple<Arguments...>;
            template <std::size_t N>
            using argument = typename std::tuple_element_t<N, std::tuple<Arguments...>>;

            constexpr static bool is_member_function_v = true;
            constexpr static bool is_class_v = std::is_class_v<Function>; 
            constexpr static bool is_const_v = function_attr::template contains<tags::is_const>;
            constexpr static bool is_volatile_v = function_attr::template contains<tags::is_volatile>;
            constexpr static bool is_noexcept_v = function_attr::template contains<tags::is_no_except>;
        };

        template <typename ClassType, typename ReturnType, typename... Arguments>
        struct member_function_traits_impl<ReturnType (ClassType::*)(Arguments...) noexcept>
            : member_function_traits_impl<
                  ReturnType (ClassType::*)(Arguments...),
                  type_tag::container<tags::is_no_except>> {};
        template <typename ClassType, typename ReturnType, typename... Arguments>
        struct member_function_traits_impl<ReturnType (ClassType::*)(Arguments...) const>
            : member_function_traits_impl<
                  ReturnType (ClassType::*)(Arguments...),
                  type_tag::container<tags::is_const>> {};
        template <typename ClassType, typename ReturnType, typename... Arguments>
        struct member_function_traits_impl<ReturnType (ClassType::*)(Arguments...) const noexcept>
            : member_function_traits_impl<
                  ReturnType (ClassType::*)(Arguments...),
                  type_tag::container<tags::is_const, tags::is_no_except>> {};
        template <typename ClassType, typename ReturnType, typename... Arguments>
        struct member_function_traits_impl<ReturnType (ClassType::*)(Arguments...) volatile>
            : member_function_traits_impl<
                  ReturnType (ClassType::*)(Arguments...),
                  type_tag::container<tags::is_volatile>> {};
        template <typename ClassType, typename ReturnType, typename... Arguments>
        struct member_function_traits_impl<ReturnType (ClassType::*)(Arguments...) const volatile>
            : member_function_traits_impl<
                  ReturnType (ClassType::*)(Arguments...),
                  type_tag::container<tags::is_const, tags::is_volatile>> {};
        template <typename ClassType, typename ReturnType, typename... Arguments>
        struct member_function_traits_impl<ReturnType (ClassType::*)(Arguments...) volatile noexcept>
            : member_function_traits_impl<
                  ReturnType (ClassType::*)(Arguments...),
                  type_tag::container<tags::is_volatile, tags::is_no_except>> {};
        template <typename ClassType, typename ReturnType, typename... Arguments>
        struct member_function_traits_impl<ReturnType (ClassType::*)(Arguments...) const volatile noexcept>
            : member_function_traits_impl<
                  ReturnType (ClassType::*)(Arguments...),
                  type_tag::container<tags::is_const, tags::is_volatile, tags::is_no_except>> {};

        template <typename, class function_attr = type_tag::container<>>
        struct function_traits_impl;
        template <typename ReturnType, typename... Arguments, class function_attr>
        struct function_traits_impl<ReturnType (*)(Arguments...), function_attr> {
            using result_type = ReturnType;
            using arguments = std::tuple<Arguments...>;
            template <std::size_t N>
            using argument = typename std::tuple_element_t<N, std::tuple<Arguments...>>;

            constexpr static bool is_member_function_v = false;
            constexpr static bool is_class_v = std::is_class_v<Function>; 
            constexpr static bool is_const_v = function_attr::template contains<tags::is_const>;
            constexpr static bool is_volatile_v = function_attr::template contains<tags::is_volatile>;
            constexpr static bool is_noexcept_v = function_attr::template contains<tags::is_no_except>;
        };
        template <typename ReturnType, typename... Arguments>
        struct function_traits_impl<ReturnType (*)(Arguments...) noexcept>
            : function_traits_impl<
                  ReturnType (*)(Arguments...),
                  type_tag::container<tags::is_no_except>> {};

#ifdef _MSC_VER
#pragma warning(default : 4348)
#endif

      public:
        using type = std::conditional_t<
            std::is_member_function_pointer_v<Function>,
            member_function_traits_impl<Function>,
            function_traits_impl<Function>>;
    };

    // is type -> functor object (including lambdas)
    //    -> [0..N] multiple parenthesis operators
    //    -> maybe overload pattern
    //    -> what about template operator() ?

    template <typename Function>
    using function_traits_t = typename function_traits<Function>::type;
    // using function_traits = member_function_traits_impl<decltype(&Function::operator()...)>;
}

#include <array>
namespace gcl::mp::test
{
    struct toto {
        int not_const_member(char) { return 42; }
        int noexcept_not_const_member(char) noexcept { return 42; }
        int const_member(char) const { return 42; }
        int noexcept_const_member(char) const noexcept { return 42; }
        int volatile_member(char) volatile { return 42; }
        int noexcept_volatile_member(char) volatile noexcept { return 42; }
    };
    static_assert(std::is_same_v<int, function_traits_t<decltype(&toto::not_const_member)>::result_type>);
    static_assert(std::is_same_v<int, function_traits_t<decltype(&toto::noexcept_not_const_member)>::result_type>);
    static_assert(std::is_same_v<int, function_traits_t<decltype(&toto::const_member)>::result_type>);
    static_assert(std::is_same_v<int, function_traits_t<decltype(&toto::noexcept_const_member)>::result_type>);
    static_assert(std::is_same_v<int, function_traits_t<decltype(&toto::volatile_member)>::result_type>);
    static_assert(std::is_same_v<int, function_traits_t<decltype(&toto::noexcept_volatile_member)>::result_type>);

    int some_function(char) { return 42; };
    int noexcept_function(char) noexcept { return 42; };
    static_assert(std::is_same_v<char, function_traits_t<decltype(&some_function)>::argument<0>>);
    static_assert(std::is_same_v<char, function_traits_t<decltype(&noexcept_function)>::argument<0>>);

    void check_function_attributes()
    {
#if not __clang__
        {
            constexpr auto check_attr = []<typename T, std::array<bool, 3> attr_expectations>() consteval
            {
                static_assert(std::get<0>(attr_expectations) == function_traits_t<T>::is_const_v);
                static_assert(std::get<1>(attr_expectations) == function_traits_t<T>::is_noexcept_v);
                static_assert(std::get<2>(attr_expectations) == function_traits_t<T>::is_volatile_v);
                return true; // symbol deduction
            };
            constexpr static auto _ = std::array{
                // ensure consteval context
                check_attr
                    .template operator()<decltype(&toto::const_member), std::array<bool, 3>{true, false, false}>(),
                check_attr
                    .template operator()<decltype(&toto::not_const_member), std::array<bool, 3>{false, false, false}>(),
                check_attr.template
                operator()<decltype(&toto::noexcept_not_const_member), std::array<bool, 3>{false, true, false}>(),
                check_attr.template
                operator()<decltype(&toto::noexcept_const_member), std::array<bool, 3>{true, true, false}>(),
                check_attr
                    .template operator()<decltype(&toto::volatile_member), std::array<bool, 3>{false, false, true}>(),
                check_attr.template
                operator()<decltype(&toto::noexcept_volatile_member), std::array<bool, 3>{false, true, true}>()};
        }
    }
    #endif
}