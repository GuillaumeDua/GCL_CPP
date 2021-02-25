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
    concept have_common_type = requires()
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
    template <typename... Ts>
    consteval auto deduplicate(const Ts... values_arg)
    {
        auto values = std::array{values_arg...};

        // deduplication
        std::sort(std::begin(values), std::end(values));
        for (auto it = std::ranges::adjacent_find(std::begin(values), std::end(values)); it != std::ranges::end(values);
             it = std::ranges::adjacent_find(std::next(it), std::end(values)))
        {
            auto end = it;
            while (end != std::end(values) && *end == *it)
                ++end;
            std::copy(end, std::end(values), std::next(it));
        }
        // std::unique()
        auto sort_end = [&values]() consteval
        {
            std::size_t i{1};
            for (; i < std::size(values) and values.at(i - 1) < values.at(i); ++i)
                ;
            return i;
        }
        ();

        return std::pair{values, sort_end};
    }
    template <std::size_t end_index, typename T, std::size_t N>
    consteval auto shrink(std::array<T, N> value)
    {
        using return_type = std::array<T, end_index>;
        return_type return_value;
        std::copy(std::cbegin(value), std::cbegin(value) + end_index, std::begin(return_value));
        return return_value;
    }
}

namespace gcl::ctc::tests::deduplicate
{
    // MSVC/CL does not consider deduplicate calls as constexpr ...
    // static_assert(gcl::ctc::deduplicate<'b', 'a', 'c', 'a', 'c', 'b', 'b'>() == std::array{'a', 'b', 'c'});

    static void by_values() {

        constexpr auto values = []() consteval
        {
            constexpr auto deduplicate_result = gcl::ctc::deduplicate(1, 2, 1, 2, 3, 4, 5, 1, 2, 3);
            constexpr auto deduplicated_values =
                std::get<0>(deduplicate_result); // structure-binding not allowed in constant-expression
            constexpr auto last_unique = std::get<1>(deduplicate_result);
           return gcl::ctc::shrink<last_unique>(deduplicated_values);
        }
        ();
        static_assert(std::is_same_v<decltype(values), const decltype(std::array{1, 2, 3, 4, 5})>);
        static_assert(values == std::array{1, 2, 3, 4, 5});
    }
}