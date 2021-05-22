#pragma once

#include <gcl/mp/type_traits.hpp>

#include <type_traits>
#include <utility>

namespace gcl::mp::meta
{
    template <template <typename...> class T, typename... Ts>
    class pack_as {

        template <typename... types>
        struct impl {
            using type = T<types...>;
        };
        template <template <typename...> class pack_t, typename... types>
        struct impl<pack_t<types...>> {
            using type = T<types...>;
        };

      public:
        using type = typename impl<Ts...>::type;
    };
    template <template <typename...> class T, typename... Ts>
    using pack_as_t = typename pack_as<T, Ts...>::type;

    template <typename T>
    requires(gcl::mp::type_traits::is_template_v<T>) struct size;
    template <template <typename...> typename T, typename... Ts>
    struct size<T<Ts...>> {
        constexpr static auto value = sizeof...(Ts);
    };
    template <typename T>
    constexpr auto size_v = size<T>::value;

    template <typename... Ts>
    requires(sizeof...(Ts) not_eq 0) struct first;
    template <typename T, typename... Ts>
    struct first<T, Ts...> {
        using type = T;
    };
    template <typename ... Ts>
    using first_t = typename first<Ts...>::type;

    template <typename T, typename ...>
    requires(gcl::mp::type_traits::is_template_v<T>)
    class append;
    template <template <typename...> typename T, typename ... Ts, typename ... args_t>
    class append<T<Ts...>, args_t...> {
    public:
        using type = T<Ts..., args_t...>;
    };
    template <typename T, typename ... args_t>
    using append_t = typename append<T, args_t...>::type;

    // todo : refactor this shitty impl ... =)
    template <typename... Ts>
    requires(sizeof...(Ts) not_eq 0) class join {
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

    // todo : split at index

    template <typename T, template <typename> typename predicate>
    class filter;
    template <template <typename...> typename Type, typename... Ts, template <typename> typename predicate>
    class filter<Type<Ts...>, predicate> {

        template <typename T>
        using element_type = std::conditional_t<predicate<T>::value, Type<>, Type<T>>;

      public:
        using type = join_t<Type<>, element_type<Ts>...>;
    };
    template <typename T, template <typename> typename predicate>
    using filter_t = typename filter<T, predicate>::type;

    template <typename T, typename... to_remove>
    class remove {
        template <typename candidate>
        struct listed_as_to_remove {
            constexpr static auto value = ((std::is_same_v<candidate, to_remove> or ...));
        };

      public:
        using type = filter_t<T, listed_as_to_remove>;
    };
    template <typename T, typename... to_remove>
    using remove_t = typename remove<T, to_remove...>::type;

    template <typename... Ts>
    class type_sequence {

        constexpr static auto size = sizeof...(Ts);
        constexpr static auto empty = (size == 0);
        // todo : at / get <index>

        constexpr type_sequence() = default;

        // todo : remove_if<type_trait_as_predicate> => filter
        // todo : `- remove_duplicates => previous position
        // todo : conjunction, disjunction
        // todo : split

      public:
        template <typename... arguments_t>
        using add = join_t<type_sequence<Ts...>, pack_as_t<type_sequence, arguments_t>...>;
        template <typename... to_remove>
        using remove = remove_t<type_sequence<Ts...>, to_remove...>; // todo : flatten / repack
    };

    // template <typename ... Ts, typename ...Us>
    // constexpr auto operator+(type_sequence<Ts...>, type_sequence<Us...>)
    //{
    //    return type_sequence<Ts...>::template add<Us...>;
    //}
    // template <typename... Ts, typename... Us>
    // constexpr auto operator-(type_sequence<Ts...>, type_sequence<Us...>)
    //{
    //    return type_sequence<Ts...>::template remove<Us...>;
    //}
}

namespace gcl::mp::tests::meta
{
    template <typename... Ts>
    struct type_seq {};

    using namespace gcl::mp::meta;

    namespace pack_as
    {
        static_assert(std::is_same_v<type_seq<int, char>, pack_as_t<type_seq, int, char>>);
        static_assert(std::is_same_v<type_seq<int, char>, pack_as_t<type_seq, type_seq<int, char>>>);
    }
    namespace size
    {
        static_assert(size_v<type_seq<>> == 0);
        static_assert(size_v<type_seq<int>> == 1);
        static_assert(size_v<type_seq<int, char>> == 2);
    }
    namespace first
    {
        static_assert(std::is_same_v<int, first_t<int, char>>);
        static_assert(not std::is_same_v<int, first_t<char>>);
    }
    namespace append
    {
        static_assert(std::is_same_v<append_t<type_seq<int>, float>, type_seq<int, float>>);
        static_assert(std::is_same_v<append_t<type_seq<int>, type_seq<float>>, type_seq<int, type_seq<float>>>);
    }
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
    namespace filters
    {
        template <typename T>
        using is_int = std::is_same<T, int>;
        static_assert(std::is_same_v<type_seq<char>, filter<type_seq<int, char>, is_int>::type>);
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
        // static_assert(std::is_same_v<type_sequence<int>, type_sequence<int, char>::remove<type_sequence<char>>>);
    }
}