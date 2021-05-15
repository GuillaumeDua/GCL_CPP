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

    template <typename... Ts>
    class type_sequence {

        template <typename T>
        struct flatten {
            using type = type_sequence<T>;
        };
        template <typename... argument_types>
        struct flatten<type_sequence<argument_types...>> {
            using type = type_sequence<argument_types...>;
        };
        template <typename T>
        using flatten_t = typename flatten<T>::type;

        // todo : remove<types...>
        // todo : remove_if<type_trait_as_predicate>
        // todo : `- remove_duplicates

      public:
        template <typename... arguments_t>
        using add = join_t<type_sequence<Ts...>, flatten_t<arguments_t>...>;
    };
}

namespace gcl::mp::tests::meta
{
    template <typename ... Ts>
    struct type_seq {};

    using namespace gcl::mp::meta;

    namespace join
    {
        static_assert(std::is_same_v<type_seq<int, char, double>, join_t<type_seq<int>, type_seq<char, double>>>);
        static_assert(std::is_same_v<type_seq<int, char, double>, join_t<type_seq<int, char, double>>>);
        static_assert(
            std::is_same_v<type_seq<int, char, double>, join_t<type_seq<int>, type_seq<char>, type_seq<double>>>);
        static_assert(std::is_same_v<
                      type_seq<int, char, double, float>,
                      join_t<type_seq<int>, type_seq<char>, type_seq<double>, type_seq<float>>>);
    }
    namespace type_sequence_t
    {
        static_assert(std::is_same_v<type_sequence<int, char>, type_sequence<int>::add<char>>);
        static_assert(std::is_same_v<type_sequence<int, char>, type_sequence<int>::add<type_sequence<char>>>);
        static_assert(std::is_same_v<type_sequence<int, char>, type_sequence<int, char>::add<type_sequence<>>>);
        static_assert(std::is_same_v<
                      type_sequence<int, char, double, bool>,
                      type_sequence<int>::add<type_sequence<>>::add<type_sequence<char>>::add<
                          type_sequence<double>>::add<bool>>);
        static_assert(
            std::is_same_v<type_sequence<int, char, double>, type_sequence<int>::add<type_sequence<char>, double>>);
        static_assert(std::is_same_v<
                      type_sequence<int, char, double, float, bool>,
                      type_sequence<int>::add<type_sequence<>>::add<
                          type_sequence<char>>::add<type_sequence<double>, float>::add<bool>>);
    }
}