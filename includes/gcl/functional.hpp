#pragma once

#include <utility>

namespace gcl::functional
{
    template <class... Ts>
    struct overload : Ts... {
        using Ts::operator()...;
        using bases_types = std::tuple<Ts...>; // allow operator() function_traits
    };
    template <class... Ts>
    overload(Ts...) -> overload<Ts...>; // not required in C++20, but Msvc need this deduction guide for some reasons ...

    template <typename T>
    decltype(auto) wrap(T&& func)
    { // wrap any functor into a deductible type/value
        return [func](auto&&... args) -> decltype(auto) {
            return std::invoke(func, std::forward<decltype(args)>(args)...);
        };
    }
}

#include <gcl/mp/function_traits.hpp>

namespace gcl::functional::type_traits
{
    template <class>
    struct is_overload : std::false_type {};
    template <class... Ts>
    struct is_overload<gcl::functional::overload<Ts...>> : std::true_type {};
    template <class... Ts>
    constexpr auto is_overload_v = is_overload<Ts...>::value;

    template <typename T>
    struct overload_arguments;
    template <typename... Ts>
    struct overload_arguments<gcl::functional::overload<Ts...>> {
        using types = std::tuple<typename gcl::mp::function_traits_t<decltype(&Ts::operator())>::arguments...>;
        using concatenated_types =
            decltype(std::tuple_cat(typename gcl::mp::function_traits_t<decltype(&Ts::operator())>::arguments{}...));
    };
    template <typename T>
    using overload_arguments_t = typename overload_arguments<T>::types;
    template <typename T>
    using overload_concatenated_arguments_t = typename overload_arguments<T>::concatenated_types;
}

#if defined(GCL_ENABLE_COMPILE_TIME_TESTS)
#include <functional>
namespace gcl::functional::tests::type_traits
{
    static_assert(gcl::functional::type_traits::is_overload_v<gcl::functional::overload<>>);
    static_assert(not gcl::functional::type_traits::is_overload_v<std::function<void()>>);

    #if defined(__clang__)
    [[maybe_unused]] auto overload_value = gcl::functional::overload{[](int) {}, []() {}, [](char, double) {}};
    using overload_type = decltype(overload_value);
    #else
    using overload_type = decltype(gcl::functional::overload{[](int) {}, []() {}, [](char, double) {}});
    #endif

    using overload_type_arguments_t = gcl::functional::type_traits::overload_arguments_t<overload_type>;
    static_assert(std::tuple_size_v<overload_type_arguments_t> == 3);
    static_assert(std::is_same_v<std::tuple_element_t<0, overload_type_arguments_t>, std::tuple<int>>);
    static_assert(std::is_same_v<std::tuple_element_t<1, overload_type_arguments_t>, std::tuple<>>);
    static_assert(std::is_same_v<std::tuple_element_t<2, overload_type_arguments_t>, std::tuple<char, double>>);

    using overload_type_concatenated_arguments_t =
        gcl::functional::type_traits::overload_concatenated_arguments_t<overload_type>;
    static_assert(std::tuple_size_v<overload_type_concatenated_arguments_t> == 3);
    static_assert(std::is_same_v<std::tuple_element_t<0, overload_type_concatenated_arguments_t>, int>);
    static_assert(std::is_same_v<std::tuple_element_t<1, overload_type_concatenated_arguments_t>, char>);
    static_assert(std::is_same_v<std::tuple_element_t<2, overload_type_concatenated_arguments_t>, double>);
}
#endif