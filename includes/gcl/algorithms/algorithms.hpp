#pragma once

#include <gcl/algorithms/ranges.hpp>
#include <gcl/algorithms/maths.hpp>

#include <functional>
#include <iterator>
#include <algorithm>
#include <concepts>

namespace gcl::algorithms
{
    // Similar to `std::for_each` by defaut
    // but add the ability to project iterator (not values)
    // for values, simply use common projections
    template <class iterator_type, class UnaryFunction, class Projection = decltype([](iterator_type it) {
                                                            return *it;
                                                        })>
    // requires
    //     std::invocable<Projection, iterator_type> and
    //     std::invocable<UnaryFunction, std::add_rvalue_reference_t<std::invoke_result_t<Projection, iterator_type>>>

    constexpr UnaryFunction for_each_it_projection(
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
        if (it == std::end(container)) // avoid precondition
            return std::pair{it, it};
        return std::pair{
            it == std::begin(container) ? std::end(container) : std::prev(it),
            it == std::end(container) ? it : std::next(it)};
    }
}