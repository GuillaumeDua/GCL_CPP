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

namespace gcl::mp::tests::meta
{
    template <typename ... Ts>
    struct type_seq {};

    using namespace gcl::mp::meta;
    static_assert(std::is_same_v<type_seq<int, char, double>, join_t<type_seq<int>, type_seq<char, double>>>);
    static_assert(std::is_same_v<type_seq<int, char, double>, join_t<type_seq<int, char, double>>>);
    static_assert(
        std::is_same_v<type_seq<int, char, double>, join_t<type_seq<int>, type_seq<char>, type_seq<double>>>);
    static_assert(std::is_same_v<
                  type_seq<int, char, double, float>,
                  join_t<type_seq<int>, type_seq<char>, type_seq<double>, type_seq<float>>>);
}