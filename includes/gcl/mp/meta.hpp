#pragma once

namespace gcl::mp::meta
{
    template <typename ... Ts>
    requires(sizeof...(Ts) not_eq 0)
    class join {
        template <typename...>
        struct impl;
        template <typename first_type, typename second_type, typename... types>
        struct impl<first_type, second_type, types...> {
            using type = typename join<typename join<first_type, second_type>::type, types...>::type;
        };
        template <typename T>
        struct impl<T> {
            using type = T;
        };
        template <template <typename...> typename Type, typename... Us, typename... Vs>
        struct impl<Type<Us...>, Type<Vs...>> {
            using type = Type<Us..., Vs...>;
        };

      public:
          using type = typename impl<Ts...>::type;
    };
    template <typename... Ts>
    using join_t = typename join<Ts...>::type;

    template <typename T, typename... to_remove> 
    class remove;
    template <template <typename ...> typename Type, typename ... Ts, typename ... to_remove>
    class remove<Type<Ts...>, to_remove...> {
        template <typename T>
        using element_type = std::conditional_t<((std::is_same_v<T, to_remove> || ...)), Type<>, Type<T>>;

      public:
        using type = join_t<Type<>, element_type<Ts>...>;
    };
    template <template <typename...> typename Type, typename... Ts, typename... to_remove>
    class remove<Type<Ts...>, Type<to_remove...>> {
        template <typename T>
        using element_type = std::conditional_t<((std::is_same_v<T, to_remove> || ...)), Type<>, Type<T>>;

      public:
        using type = join_t<Type<>, element_type<Ts>...>;
    };
    template <typename T, typename ... to_remove>
    using remove_t = typename remove<T, to_remove...>::type;

    template <typename... Ts>
    class type_sequence {

        constexpr type_sequence() = default;

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

        // todo : remove_if<type_trait_as_predicate> => filter
        // todo : `- remove_duplicates => previous position
        // todo : conjunction, disjunction

      public:
        template <typename... arguments_t>
        using add = join_t<type_sequence<Ts...>, flatten_t<arguments_t>...>;
        template <typename... to_remove>
        using remove = remove_t<type_sequence<Ts...>, to_remove...>;
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
    namespace remove
    {
        static_assert(std::is_same_v<type_seq<>, remove_t<type_seq<>>>);
        static_assert(std::is_same_v<type_seq<int>, remove_t<type_seq<int, char>, char>>);
        static_assert(std::is_same_v<type_seq<int>, remove_t<type_seq<int, char>, char, char>>);
    }
    namespace type_sequence_t::add
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
    namespace type_sequence_t::remove
    {
        static_assert(std::is_same_v<type_sequence<int>, type_sequence<int, char>::remove<char>>);
        static_assert(std::is_same_v<type_sequence<>, type_sequence<>::remove<char>>);
        static_assert(std::is_same_v<type_sequence<int>, type_sequence<int, char>::remove<type_sequence<char>>>);
    }
}