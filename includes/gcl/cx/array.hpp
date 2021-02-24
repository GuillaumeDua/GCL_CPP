#pragma once

#include <gcl/mp/type_traits.hpp>

#include <array>
#include <algorithm>
#include <type_traits>
#include <ranges>

namespace gcl::cx::concepts
{
    template <typename T>
    concept array_value = gcl::mp::type_traits::is_std_array_v<T> or std::is_array_v<T>;
}
namespace gcl::cx::array
{
    template <concepts::array_value auto arg>
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
    template <concepts::array_value auto arg>
    constexpr static auto remove_duplicates_v = remove_duplicates<arg>();
    template <concepts::array_value auto arg>
    using remove_duplicates_t = typename std::decay_t<decltype(remove_duplicates_v<arg>)>;

    template <typename... Ts>
    consteval auto to_array(Ts&&... values)
    {
        using element_type = std::common_type_t<Ts...>;
        using array_type = std::array<element_type, sizeof...(values)>;
        return array_type{std::forward<decltype(values)>(values)...};
    }
    template <auto... values>
    consteval auto to_array()
    {
        using element_type = std::common_type_t<decltype(values)...>;
        using array_type = std::array<element_type, sizeof...(values)>;
        return array_type{std::forward<decltype(values)>(values)...};
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

namespace gcl::cx::tests::array
{
    constexpr auto datas = std::array{'a', 'b', 'a', 'c', 'a'};
    static_assert(std::is_same_v<std::array<char, 3>, gcl::cx::array::remove_duplicates_t<datas>>);
    static_assert(gcl::cx::array::remove_duplicates_v<datas> == std::array{'a', 'b', 'c'});
    static_assert(gcl::cx::array::to_array('a', 'b', 'c') == std::array{'a', 'b', 'c'});
    static_assert(gcl::cx::array::to_array<'a', 'b', 'c'>() == std::array{'a', 'b', 'c'});
}