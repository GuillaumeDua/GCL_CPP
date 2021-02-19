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
#include <bitset>
#include <tuple>
#include <array>
namespace gcl::mp::type_traits
{
    template <template <typename> class trait, typename... Ts>
    struct trait_result {
        constexpr static auto as_bitset_v = []() consteval
        {
            using bitset_type = std::bitset<sizeof...(Ts)>;
            using bitset_initializer_t = unsigned long long;
            bitset_initializer_t value{0};
            return bitset_type{((value = (value << 1 | trait<Ts>::value)), ...)};
        }
        ();

        template <template <typename...> class T>
        using as_t = T<typename trait<Ts>::type...>;
        template <template <typename...> class T>
        constexpr static auto as_v = T{trait<Ts>::value...};

        using as_tuple_t = as_t<std::tuple>;
        constexpr static auto as_tuple_v = std::tuple{trait<Ts>::value...};

        template <class Int = int>
        using as_integer_sequence = std::integer_sequence<Int, trait<Ts>::value...>;

        // constexpr static auto as_array_v = std::array{trait<Ts>::value...}; // OK with GCC 10.2
        using as_array_t = std::array<std::tuple_element_t<0, decltype(std::tuple{trait<Ts>::value...})>, sizeof...(Ts)>;
        constexpr static auto as_array_v = as_array_t{trait<Ts>::value...};
    };
}
namespace gcl::mp::type_trait
{   // detection not covered by `is_instance_of`
    template <typename T>
    struct is_std_array : std::false_type {};
    template <typename T, std::size_t N>
    struct is_std_array<std::array<T, N>> : std::true_type {};
    template <typename T>
    using is_std_array_t = is_std_array<T>::type;
    template <typename T>
    constexpr static auto is_std_array_v = is_std_array_t<T>::value;
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
namespace gcl::mp::type_traits::tests::trait_results
{
    template <typename T>
    using is_int = std::is_same<int, T>;
    using results = trait_result<is_int, char, int, bool>;

    // static_assert(decltype(results::as_bitset_v){2UL} == results::as_bitset_v); // std::bitset::operator== is not cx
    constexpr auto expected_result_as_bitset = decltype(results::as_bitset_v){2UL};
    static_assert(expected_result_as_bitset[0] == results::as_bitset_v[0]);
    static_assert(expected_result_as_bitset[1] == results::as_bitset_v[1]);
    static_assert(expected_result_as_bitset[2] == results::as_bitset_v[2]);

    using results_as_tuple = results::as_t<std::tuple>;
    using results_as_tuple_value_type = std::decay_t<decltype(results::as_v<std::tuple>)>;
    static_assert(std::tuple_size_v<results_as_tuple> == std::tuple_size_v<results_as_tuple_value_type>);

    using expected_result_as_tuple = std::tuple<std::false_type, std::true_type, std::false_type>;
    static_assert(std::is_same_v<results::as_t<std::tuple>, expected_result_as_tuple>);
    static_assert(std::is_same_v<results::as_integer_sequence<int>, std::integer_sequence<int, 0, 1, 0>>);
    static_assert(results::as_array_v == std::array{false, true, false});

    template <typename... Ts>
    struct type_pack {};
    using results_as_type_pack = results::as_t<type_pack>;
    using expected_result_as_type_pack = type_pack<std::false_type, std::true_type, std::false_type>;
    static_assert(std::is_same_v<results_as_type_pack, expected_result_as_type_pack>);
}