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

    template <typename container_type, typename iterator_type>
    [[nodiscard]] constexpr auto adjacent(container_type& container, iterator_type it) noexcept
    -> std::pair<iterator_type, iterator_type>
    {
        if (it == std::cend(container))
            return std::pair{it, it};
        return std::pair{
            it == std::begin(container) ? std::end(container) : std::prev(it),
            it == std::end(container) ? it : std::next(it)};
    }
    template <typename container_type, typename iterator_type, typename Predicate>
    [[nodiscard]] constexpr auto adjacent_if(container_type & container, iterator_type it, Predicate predicate)
    noexcept(std::is_nothrow_invocable_v<Predicate, const typename container_type::value_type&>)
    -> decltype(adjacent(container, it)) {

        auto adjacent_result = adjacent(container, it);
        auto transformation = [&](auto & value){
            if (value not_eq std::end(container) and not predicate(*value))
                value = std::end(container);
        };

        [&]<std::size_t ... indexes>(std::index_sequence<indexes...>){
            static_assert((std::is_invocable_v<Predicate, decltype(std::get<indexes>(adjacent_result))> && ...));
            ((transformation(std::get<indexes>(adjacent_result))), ...);
        }(std::make_index_sequence<std::tuple_size_v<decltype(adjacent_result)>>{});
        return adjacent_result;
    }
}

#ifdef GCL_ENABLE_COMPILE_TIME_TESTS
#include <vector>
namespace gcl::algorithms::tests
{
    consteval void for_each_(){

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
    consteval void adjacent_() {
        using namespace gcl::algorithms;
        {   // adjacent(container, const_iterator)
            constexpr auto datas = std::array{1,2,3,4};
            static_assert(
                adjacent(datas, std::cbegin(datas)) ==
                std::pair{ std::cend(datas), std::next(std::cbegin(datas)) }
            );
            static_assert(
                adjacent(datas, std::next(std::cbegin(datas))) ==
                std::pair{ std::cbegin(datas), std::next(std::cbegin(datas), 2) }
            );
        }
        { // adjacent(container, iterator)
        }
        { // adajacent_if(container, const_iterator, Predicate)

        }
        { // adajacent_if(container, iterator, Predicate)

        }
    }
}
#endif