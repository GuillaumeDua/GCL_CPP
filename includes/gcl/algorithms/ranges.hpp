#pragma once

#include <iterator>
#include <type_traits>
#include <ranges>
#include <stdexcept>

namespace gcl::algorithms::ranges
{
    template <typename RangeIterator, typename InputIterator>
    constexpr bool is_in_range(RangeIterator range_begin, RangeIterator range_end, InputIterator input)
    {
        using range_category = typename std::iterator_traits<RangeIterator>::iterator_category;
        static_assert(std::is_base_of_v<std::input_iterator_tag, range_category>);
        using input_category = typename std::iterator_traits<RangeIterator>::iterator_category;
        static_assert(std::is_base_of_v<std::input_iterator_tag, input_category>);

        if constexpr (
            std::is_base_of_v<std::random_access_iterator_tag, range_category> and
            std::is_base_of_v<std::random_access_iterator_tag, input_category>)
        {
            if (range_begin > range_end)
                throw std::out_of_range{"gcl::algorithms::ranges::is_in_range"};
            return input >= range_begin and input <= range_end;
        }
        else
        {   // UB if range_begin > range_end
            static_assert(std::equality_comparable_with<RangeIterator, InputIterator>);
            while (range_begin != range_end)
            {
                if (input == range_begin)
                    return true;
                ++range_begin;
            }
            return false;
        }
    }
    template <std::ranges::range Range, typename InputIterator>
    constexpr bool is_in_range(const Range & range_value, InputIterator input)
    {
        return is_in_range(std::begin(range_value), std::end(range_value), input);
    }
}

#if defined(GCL_ENABLE_RUNTIME_TESTS)
#include <map>
#include <algorithm>
namespace gcl::algorithms::tests::ranges
{
    void is_in_range() {
        auto map_value = std::map<int, char>{};
        std::generate_n(std::inserter(map_value, std::end(map_value)), 9, [i = 0]() mutable {
            return decltype(map_value)::value_type{i, i++};
        });
        if (not gcl::algorithms::ranges::is_in_range(std::cbegin(map_value), std::cend(map_value), map_value.find(5)))
            throw std::runtime_error{""};
        if (gcl::algorithms::ranges::is_in_range(std::cbegin(map_value), std::cend(map_value), std::cend(map_value)))
            throw std::runtime_error{""};
    }
}
namespace gcl::algorithms::tests::ranges
{
    void test() { is_in_range();
    }
}
#endif