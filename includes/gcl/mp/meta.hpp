#pragma once

namespace gcl::mp::meta
{
    template <typename... Ts>
    struct join;
    template <typename first_type, typename second_type, typename... Ts>
    struct join<first_type, second_type, Ts...> {
        static_assert(sizeof...(Ts) not_eq 0);
        using type = typename join<typename join<first_type, second_type>::type, Ts...>::type;
    };
    template <typename T>
    struct join<T> {
        using type = T;
    };
    template <template <typename...> typename Type, typename... Ts, typename... Us>
    struct join<Type<Ts...>, Type<Us...>> {
        using type = Type<Ts..., Us...>;
    };

    template <typename... Ts>
    using join_t = typename join<Ts...>::type;
}

#include <tuple>

namespace gcl::mp::tests::meta
{
    using namespace gcl::mp::meta;
    static_assert(std::is_same_v<std::tuple<int, char, double>, join_t<std::tuple<int>, std::tuple<char, double>>>);
    static_assert(std::is_same_v<std::tuple<int, char, double>, join_t<std::tuple<int, char, double>>>);
    static_assert(
        std::is_same_v<std::tuple<int, char, double>, join_t<std::tuple<int>, std::tuple<char>, std::tuple<double>>>);
    static_assert(std::is_same_v<
                  std::tuple<int, char, double, float>,
                  join_t<std::tuple<int>, std::tuple<char>, std::tuple<double>, std::tuple<float>>>);
}