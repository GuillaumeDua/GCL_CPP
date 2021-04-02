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
    constexpr static auto is_overload_v = is_overload<Ts...>::value;

    template <typename T>
    struct overload_arguments;
    template <typename ... Ts>
    struct overload_arguments<gcl::functional::overload<Ts...>> {
        // std::tuple<std::tuple<args1...>, std::tuple<args2...>, etc.>
        using types = std::tuple<typename gcl::mp::function_traits_t<decltype(&Ts::operator())>::arguments...>;
        // std::tuple<args1..., args2..., etc.>
        using concatenated_types =
            decltype(std::tuple_cat(typename gcl::mp::function_traits_t<decltype(&Ts::operator())>::arguments{}...));
    };
    template <typename T>
    using overload_arguments_t = typename overload_arguments<T>::type;
}

#include <functional>

namespace gcl::functional::tests::type_traits
{
    static_assert(gcl::functional::type_traits::is_overload_v<gcl::functional::overload<>>);
    static_assert(not gcl::functional::type_traits::is_overload_v<std::function<void()>>);
}