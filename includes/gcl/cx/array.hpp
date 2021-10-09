#pragma once

#include <array>
#include <algorithm>
#include <type_traits>

#if defined(__clang__)
# pragma message("[gcl] in " __FILE__ " : clang/clang-cl does not support ranges yet")
#elif not defined(__cpp_lib_ranges)
# pragma message("[gcl] in " __FILE__ " : missing __cpp_lib_ranges")
#else
# include <ranges>
#endif

namespace gcl::mp::type_traits
{ // detection not covered by `is_instance_of`
    template <typename T>
    struct is_std_array : std::false_type {};
    template <typename T, std::size_t N>
    struct is_std_array<std::array<T, N>> : std::true_type {};
    template <typename T>
    using is_std_array_t = typename is_std_array<T>::type;
    template <typename T>
    constexpr auto is_std_array_v = is_std_array_t<T>::value;
}
namespace gcl::concepts
{
    template <typename T>
    concept StdArray = gcl::mp::type_traits::is_std_array_v<T>;
    template <typename T>
    concept RawArray = std::is_array_v<T>;
    template <typename T>
    concept Array = StdArray<T> or RawArray<T>;
}
namespace gcl::cx::array::literals
{
    template <typename T, std::size_t N>
    struct std_array_value {
        using value_type = std::array<T, N>;
        value_type value;

        constexpr std_array_value(value_type&& arg)
            : value{std::forward<value_type>(arg)}
        {}
        constexpr std_array_value(T const (&arg)[N + 1])
        {
            std::copy_n(std::begin(arg), std::size(value), std::begin(value));
        };

        operator value_type&() { return value; }
        operator const value_type&() const { return value; }
        operator value_type&&() { return std::move(value); }
    };
    template <typename T, std::size_t N>
    std_array_value(T const (&arg)[N]) -> std_array_value<T, N - 1>;
    template <typename T, std::size_t N>
    std_array_value(std::array<T, N>) -> std_array_value<T, N>;

    #if defined(_MSC_VER) and defined(__clang__)
    # pragma message("[gcl] in " __FILE__ " : literal _std_array with char... disabled for clang-cl")
    #else
    template <std_array_value str_arg>
    constexpr auto operator"" _std_array()
    {
        return str_arg.value;
    }
    #endif

    template <char... values>
    constexpr auto operator"" _std_array()
    {
        return std::array{values...};
    }
}
namespace gcl::cx::array
{
    template <concepts::Array auto arg>
    consteval static auto remove_duplicates()
    {
        using argument_type = decltype(arg);
        using value_type = typename argument_type::value_type;

        constexpr auto size = []() consteval
        {
            auto copy = arg;
            std::ranges::sort(copy);
            return std::ranges::size(arg) - std::ranges::size(std::ranges::unique(copy));
        }
        ();

        auto copy = arg;
        std::ranges::sort(copy);
        auto last_not_duplicate = std::unique(std::begin(copy), std::end(copy));

        using return_type = std::array<value_type, size>;
        return_type return_value;
        std::copy(std::begin(copy), last_not_duplicate, std::begin(return_value));
        return return_value;
    };
    template <concepts::Array auto arg>
    constexpr auto remove_duplicates_v = remove_duplicates<arg>();
    template <concepts::Array auto arg>
    using remove_duplicates_t = typename std::decay_t<decltype(remove_duplicates_v<arg>)>;

    template <typename... Ts>
    consteval auto to_array(Ts&&... values)
    {
        using element_type = std::common_type_t<Ts...>;
        using Array = std::array<element_type, sizeof...(values)>;
        return Array{std::forward<decltype(values)>(values)...};
    }
    template <auto... values>
    consteval auto to_array()
    {
        using element_type = std::common_type_t<decltype(values)...>;
        using Array = std::array<element_type, sizeof...(values)>;
        return Array{std::forward<decltype(values)>(values)...};
    }

    // WIP
    // template <typename T, std::size_t N, typename FunctionType>
    // requires(std::invocable<FunctionType, )
    // consteval auto to_parameter_pack(std::array<T, N> values, FunctionType&& function)
    // {
    //     return [&]<std::size_t... indexes>(std::index_sequence<indexes...>) consteval
    //     {
    //         return function(values.at(indexes)...);
    //     }
    //     (std::make_index_sequence<N>{});
    // }
}

#if defined(GCL_ENABLE_COMPILE_TIME_TESTS)
namespace gcl::mp::type_traits::tests::is_std_array
{
    static_assert(gcl::mp::type_traits::is_std_array_v<std::array<char, 3>>);
    static_assert(not gcl::mp::type_traits::is_std_array_v<char[3]>);
}
namespace gcl::cx::array::literals::tests::operator_std_array
{
    using namespace gcl::cx::array::literals;
    static_assert(1234_std_array == std::array{'1', '2', '3', '4'});

    #if defined(_MSC_VER)
    # pragma message("[gcl] in " __FILE__ " : literal _std_array with char... disabled for msvc-cl/clang-cl")
    #else
    static_assert("1234"_std_array == std::array{'1', '2', '3', '4'});    
    #endif

    constexpr auto v1 = std_array_value{"toto"}.value;
    constexpr auto v2 = std_array_value{std::array{'t', 'o', 't', 'o'}}.value;
    static_assert(v1 == v2);
}
namespace gcl::cx::tests::array
{
    constexpr auto datas = std::array{'a', 'b', 'a', 'c', 'a'};
#if (defined(__clang__)) // not Clang nor Clang-CL
#else
    static_assert(std::is_same_v<std::array<char, 3>, gcl::cx::array::remove_duplicates_t<datas>>);
    static_assert(gcl::cx::array::remove_duplicates_v<datas> == std::array{'a', 'b', 'c'});
#endif
    static_assert(gcl::cx::array::to_array('a', 'b', 'c') == std::array{'a', 'b', 'c'});
    static_assert(gcl::cx::array::to_array<'a', 'b', 'c'>() == std::array{'a', 'b', 'c'});
}
#endif