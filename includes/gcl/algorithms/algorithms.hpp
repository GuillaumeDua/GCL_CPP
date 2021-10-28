#pragma once

#include <gcl/algorithms/ranges.hpp>
#include <gcl/algorithms/maths.hpp>

#include <functional>
#include <iterator>
#include <algorithm>
#include <concepts>

namespace gcl::algorithms
{
    // todo :
    //  for_each that detected if the projection takes an iterator or value as parameter ?

    // Similar to `std::for_each` by defaut
    // but add the ability to project iterator (not values)
    // for values, simply use common projections
    template <class iterator_type, class UnaryFunction, class Projection = std::identity>
    // requires
    //     std::invocable<Projection, iterator_type> and
    //     std::invocable<UnaryFunction, std::add_rvalue_reference_t<std::invoke_result_t<Projection, iterator_type>>>

    constexpr UnaryFunction for_each_it(
        iterator_type range_begin, iterator_type range_end, UnaryFunction f, Projection proj = {})
    // noexcept(...)
    {
        static_assert(std::is_invocable_v<Projection, iterator_type>);
        using projection_result_t = std::invoke_result_t<Projection, iterator_type>;
        static_assert(std::is_invocable_v<UnaryFunction, std::add_rvalue_reference_t<projection_result_t>>);

        for (; range_begin != range_end; ++range_begin)
        {
            auto value = std::invoke(proj, range_begin);
            std::invoke(f, std::move(value));
        }
        return f;
    }

    template <typename container_type>
    constexpr auto adjacent(container_type& container, typename container_type::iterator it) noexcept
    {
        if (it == std::end(container))
            return std::pair{it, it};
        return std::pair{
            it == std::begin(container) ? std::end(container) : std::prev(it),
            it == std::end(container) ? it : std::next(it)};
    }
    template <typename container_type, typename Predicate>
    constexpr auto adjacent_if(container_type & container, typename container_type::iterator it, Predicate predicate)
    noexcept(std::is_nothrow_invocable_v<Predicate, const typename container_type::value_type&>) {

        auto [lhs, rhs] = adjacent(container, it);

        static_assert(std::is_invocable_v<Predicate, decltype(*lhs)>);
        static_assert(std::is_invocable_v<Predicate, decltype(*rhs)>);
        return std::pair{
            predicate(*lhs) ? lhs : std::end(container),
            predicate(*rhs) ? rhs : std::end(container)
        };
    }
}

#ifdef GCL_ENABLE_COMPILE_TIME_TESTS
#include <vector>
namespace gcl::algorithms::tests
{
    constexpr void for_each_(){

        constexpr auto values = std::array{'a', 'b', 'c', 'd'};
        // same as std::for_each
        gcl::algorithms::for_each_it(
            std::cbegin(values), std::cend(values), [](const auto& value) { });
        // same as std::for_each with projection on it
        gcl::algorithms::for_each_it(
            std::cbegin(values),
            std::cend(values),
            [](const auto& projected_element) { const auto& [index, value] = projected_element; },
            [range_begin = std::cbegin(values)](const auto& it) {
                return std::pair{std::distance(range_begin, it), *it};
            });
    }
}
#endif