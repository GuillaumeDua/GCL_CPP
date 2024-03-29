#pragma once

#include <type_traits>
#include <utility>
namespace gcl::mp::type_traits
{
    template <class T>
    struct is_template : std::false_type {};
    template <class... T_args, template <class...> class T>
    struct is_template<T<T_args...>> : std::true_type {};
    template <auto... values, template <auto...> class T>
    struct is_template<T<values...>> : std::true_type {};
    template <class T>
    inline constexpr auto is_template_v = is_template<T>::value;

    template <class T, class U>
    struct is_same_template_type : std::false_type {};
    template <template <class...> class T, class... T_args, class... U_args>
    struct is_same_template_type<T<T_args...>, T<U_args...>> : std::true_type {};
    template <template <auto...> class T, auto... T_values, auto... U_values>
    struct is_same_template_type<T<T_values...>, T<U_values...>> : std::true_type {};
    template <class T, class U>
    inline constexpr auto is_same_template_type_v = is_same_template_type<T, U>::value;

    template <typename T, typename = void>
    struct is_complete : std::false_type {};
    template <typename T>
    struct is_complete<T, std::void_t<decltype(sizeof(T))>>
        : std::true_type {};
    template <typename T>
    constexpr inline auto is_complete_v = is_complete<T>::value;

    template <class T_concrete, template <class...> class T>
    struct is_instance_of : std::false_type {};
    template <template <class...> class T, class... T_args>
    struct is_instance_of<T<T_args...>, T> : std::true_type {};
    template <class T_concrete, template <class...> class T>
    inline constexpr auto is_instance_of_v = is_instance_of<T_concrete, T>::value;

    template <typename T, typename U>
    struct is_same_cvref_qualifiers {
        constexpr static bool value =
            (std::is_lvalue_reference_v<T> == std::is_lvalue_reference_v<U> and
             std::is_rvalue_reference_v<T> == std::is_rvalue_reference_v<U> and
             std::is_const_v<std::remove_reference_t<T>> == std::is_const_v<std::remove_reference_t<U>> and
             std::is_volatile_v<std::remove_reference_t<T>> == std::is_volatile_v<std::remove_reference_t<U>>);
    };
    template <typename T, typename U>
    constexpr bool is_same_cvref_qualifiers_v = is_same_cvref_qualifiers<T, U>::value;

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
    constexpr inline auto is_brace_constructible_v = is_brace_constructible<T, Args...>::value;

    template <bool evaluation>
    using if_t = std::conditional_t<evaluation, std::true_type, std::false_type>;
    template <bool evaluation>
    constexpr inline auto if_v = std::conditional_t<evaluation, std::true_type, std::false_type>::value;

    template <typename...>
    struct dependent_false {
        constexpr static auto value = false;
    };
    template <typename... args>
    constexpr auto dependent_false_v = dependent_false<args...>::value;

    template <template <typename> typename first_trait, template <typename> typename... traits>
    struct merge_traits_t {
        template <typename T>
        using type = typename merge_traits_t<traits...>::template type<first_trait<T>>;
    };
    template <template <typename> typename first_trait>
    struct merge_traits_t<first_trait> {
        template <typename T>
        using type = first_trait<T>;
    };

    template <template <typename> typename first_trait, template <typename> typename... traits>
    struct merge_traits {
        template <typename T>
        using type = typename first_trait<typename merge_traits<traits...>::template type<T>>::type;
    };
    template <template <typename> typename first_trait>
    struct merge_traits<first_trait> {
        template <typename T>
        using type = typename first_trait<T>::type;
    };

    template <template <typename...> class base_type, typename... Ts>
    class partial {
        // differs type instanciation with partial template-type parameters
        template <typename... Us>
        struct impl {
            // workaround for https://gcc.gnu.org/bugzilla/show_bug.cgi?id=59498
            using type = base_type<Ts..., Us...>;
        };
        template <typename U>
        struct impl<U> {
            using type = base_type<Ts..., U>;
        };

      public:
        template <typename... Us>
        requires(sizeof...(Us) >= 1) using type = typename impl<Us...>::type;
    };
    template <template <typename...> class base_type, typename... Ts>
    struct partial_t {
        template <typename... Us>
        using type = typename partial<base_type, Ts...>::template type<Us...>::type;
    };
}

#include <bitset>
#include <tuple>
#include <array>
namespace gcl::mp::type_traits
{
    template <template <typename> class trait, typename... Ts>
    struct trait_result {
        constexpr static inline auto as_bitset_v = []() consteval
        {
            using bitset_type = std::bitset<sizeof...(Ts)>;
            using bitset_initializer_t = unsigned long long;
            bitset_initializer_t value{0};
            return bitset_type{((value = (value << 1 | trait<Ts>::value)), ...)};
        }
        ();

        // see gcl::mp::pack_traits::pack_arguments_as for other expansion/conversions
        template <template <typename...> class T>
        using as_t = T<typename trait<Ts>::type...>;
        template <template <typename...> class T>
        constexpr static inline auto as_v = T{trait<Ts>::value...};
        using as_tuple_t = as_t<std::tuple>;
        constexpr static inline auto as_tuple_v = std::tuple{trait<Ts>::value...};
        template <class Int = int>
        using as_integer_sequence = std::integer_sequence<Int, trait<Ts>::value...>;
        // constexpr static auto as_array_v = std::array{trait<Ts>::value...}; // OK with GCC 10.2
        using as_array_t = std::array<std::tuple_element_t<0, decltype(std::tuple{trait<Ts>::value...})>, sizeof...(Ts)>;
        constexpr static inline auto as_array_v = as_array_t{trait<Ts>::value...};
    };
}

#if defined(GCL_ENABLE_COMPILE_TIME_TESTS)
#include <string>
namespace gcl::mp::type_traits::tests::is_template
{
    static_assert(gcl::mp::type_traits::is_template_v<std::tuple<int, char>>);
    static_assert(gcl::mp::type_traits::is_template_v<std::string>); // std::basic_string<charT, allocator>
    static_assert(not gcl::mp::type_traits::is_template_v<int>);
}
namespace gcl::mp::type_traits::tests::is_same_template_type
{
    static_assert(gcl::mp::type_traits::is_same_template_type_v<std::tuple<int>, std::tuple<char, float>>);
}
namespace gcl::mp::type_traits::tests::is_complete
{
    struct complete_type {};
    struct incomplete_type;

    static_assert(gcl::mp::type_traits::is_complete_v<complete_type>);
    static_assert(gcl::mp::type_traits::is_complete_v<int>);
    static_assert(not gcl::mp::type_traits::is_complete_v<incomplete_type>);
}
namespace gcl::mp::type_traits::tests::is_instance_of
{
    static_assert(gcl::mp::type_traits::is_instance_of_v<std::tuple<int, char>, std::tuple>);
    static_assert(not gcl::mp::type_traits::is_instance_of_v<std::tuple<int, char>, std::pair>);
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

#include <concepts>
namespace gcl::mp::type_traits::tests::if_t
{
    // clang-format off
    template <typename T>
    concept is_red_colored = requires(T)
    {
        //{ T::color == decltype(T::color)::red } -> std::convertible_to<bool>;
        { type_traits::if_t<T::color == decltype(T::color)::red>{}} -> std::same_as<std::true_type>;
        // equivalent to : `requires (T::color == decltype(T::color)::red);`
    };
    enum colors { red, blue, green };
    struct smthg_blue { constexpr static auto color = colors::blue; };
    struct smthg_red { constexpr static auto color = colors::red; };
    // clang-format on

    static_assert(not is_red_colored<smthg_blue>);
    static_assert(is_red_colored<smthg_red>);
}

namespace gcl::mp::type_traits::tests::merge_traits_t
{
    using remove_cv_and_ref = gcl::mp::type_traits::merge_traits_t<std::remove_reference_t, std::decay_t>;
    static_assert(std::is_same_v<int, remove_cv_and_ref::type<const int&&>>);
}
namespace gcl::mp::type_traits::tests::merge_traits
{
    template <typename T>
    using add_cvref_t =
        gcl::mp::type_traits::merge_traits<std::add_lvalue_reference, std::add_const, std::add_volatile>::type<T>;
    static_assert(std::is_same_v<add_cvref_t<int>, const volatile int&>);
}
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

    #if not defined(__clang__)
    // See my Q on SO :
    //  https://stackoverflow.com/questions/66821952/clang-error-implicit-instantiation-of-undefined-template-stdtuple-sizeauto/66822584
    using results_as_tuple_value_type = std::decay_t<decltype(results::as_v<std::tuple>)>;
    static_assert(std::tuple_size_v<results_as_tuple> == std::tuple_size_v<results_as_tuple_value_type>); // clang and clang-cl complain here

    using expected_result_type = std::tuple<
        std::false_type,
        std::true_type,
        std::false_type>;
    static_assert(std::is_same_v<results_as_tuple, expected_result_type>);
    using expected_result_value_type = std::tuple<bool, bool, bool>;
    static_assert(std::is_same_v<results_as_tuple_value_type, expected_result_value_type>);
    #endif
    
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
namespace gcl::mp::type_traits::tests::dependent_false
{
    constexpr auto qwe = []<typename T>() -> bool { 
        if constexpr (true)
            ;
        else
            static_assert(gcl::mp::type_traits::dependent_false_v<T>);
        return {};
    }. template operator()<int>();
}
namespace gcl::mp::type_traits::tests::partial
{
    static_assert(gcl::mp::type_traits::partial<std::is_same, int>::type<int>::value);
    static_assert(gcl::mp::type_traits::partial<std::is_same>::type<int, int>::value);

    template <typename T>
    using add_const_t = partial_t<std::add_const>::type<T>;
    static_assert(std::is_same_v<add_const_t<int>, std::add_const_t<int>>);
}
#endif