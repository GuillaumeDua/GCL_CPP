#pragma once

#include <array>
#include <tuple>
#include <type_traits>
#include <concepts>
#include <algorithm>

// experimental compile-time-constant operations

namespace gcl::ctc::concepts
{
    template <typename ... Ts>
    concept have_common_type = requires(Ts)
    {
        std::common_type_t<Ts...>{};
    };
}

namespace gcl::ctc
{
    template <typename T, std::size_t N>
    consteval auto deduplicate_size(std::array<T, N> values_as_array)
    {
        std::ranges::sort(values_as_array);
        // auto size = std::ranges::size(values_as_array) - std::ranges::distance(std::ranges::unique(values_as_array)); // clang does not support it yet
        auto size = std::distance(
            std::begin(values_as_array), std::unique(std::begin(values_as_array), std::end(values_as_array)));
        return size;
    }
    template <typename... Ts>
        requires concepts::have_common_type<Ts...> 
    consteval auto deduplicate_size(Ts... values)
    {
        return deduplicate_size(std::array{values...});
    }
    template <auto... values>
    requires concepts::have_common_type<decltype(values)...> consteval auto deduplicate()
    {
        auto values_as_array = std::array{values...};
        std::sort(std::begin(values_as_array), std::end(values_as_array));
        for (auto it = std::ranges::adjacent_find(values_as_array); it != std::end(values_as_array);
             it = std::ranges::adjacent_find(values_as_array))
        {
            auto end = it;
            while (*end == *it)
                end++;
            std::copy(std::next(end), std::end(values_as_array), std::next(it));
        }

        constexpr auto return_size = deduplicate_size(values...);
        using element_type = std::common_type_t<decltype(values)...>;
        using return_type = std::array<element_type, return_size>;
        return_type return_value;
        std::copy(std::cbegin(values_as_array), std::cbegin(values_as_array) + return_size, std::begin(return_value));
        return return_value;
    }
}

namespace gcl::ctc::tests::deduplicate
{
    // MSVC/CL does not consider deduplicate calls as constexpr ...
    // static_assert(gcl::ctc::deduplicate<'b', 'a', 'c', 'a', 'c', 'b', 'b'>() == std::array{'a', 'b', 'c'});
}