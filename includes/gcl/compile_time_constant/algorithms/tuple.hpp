#pragma once

#include <tuple>
#include <array>
#include <algorithm>
#include <utility>
#include <type_traits>

namespace gcl::ctc::algorithms::tuple
{
#if not defined(__clang__) and defined(__GNUC__)
    // Works only on GCC
    // Currently not supported by :
    // - MSVC/CL : Request non-variable parameters capture, which is non-legal
    // - Clang   : compiler crash, need to investigate
    //             Shorter crash case sample : https://godbolt.org/z/ovW7b6

    template <typename PredicateType, auto... arguments>
    constexpr auto tuple_erase_if(const PredicateType predicate)
    {
        constexpr auto element_if_predicate = [predicate]<auto argument>() consteval
        {
            if constexpr (predicate(argument))
                return std::tuple<>{};
            else
                return std::tuple{argument};
        };
        return std::tuple_cat(element_if_predicate.template operator()<arguments>()...);
    }
    template <typename TupleType, std::size_t N>
    using tuple_type_at = typename std::tuple_element_t<N, TupleType>;
    template <auto... values>
    constexpr auto tuple_erase_duplicate_values()
    {
        constexpr auto arguments = std::tuple{values...};

        constexpr auto element_if_predicate = [arguments]<auto argument, std::size_t argument_index>() consteval
        {
            if constexpr (argument_index == 0)
                return std::tuple{argument};
            else
            {
                constexpr auto has_previous_position =
                    [&arguments]<std::size_t... indexes>(std::index_sequence<indexes...>) consteval
                {
                    return (((std::get<indexes>(arguments) == argument) || ...));
                }
                (std::make_index_sequence<argument_index>{});
                if constexpr (has_previous_position)
                    return std::tuple{};
                else
                    return std::tuple{argument};
            }
        };

        return [&arguments, element_if_predicate ]<std::size_t... indexes>(std::index_sequence<indexes...>) consteval
        {
            return std::tuple_cat(element_if_predicate.template operator()<std::get<indexes>(arguments), indexes>()...);
        }
        (std::make_index_sequence<std::tuple_size_v<decltype(arguments)>>{});
    }

    template <auto... values>
    constexpr auto tuple_erase_duplicate_types()
    {
        using values_as_tuple_type = std::tuple<decltype(values)...>;
        constexpr auto element_if_predicate = []<auto argument, std::size_t argument_index>() consteval
        {
            constexpr auto has_previous_position = []<std::size_t... indexes>(std::index_sequence<indexes...>) consteval
            {
                return (
                    (std::is_same_v<std::tuple_element_t<indexes, values_as_tuple_type>, decltype(argument)> || ...));
            }
            (std::make_index_sequence<argument_index>{});
            if constexpr (has_previous_position)
                return std::tuple<>{};
            else
                return std::tuple{argument};
        };

        return [element_if_predicate]<std::size_t... indexes>(std::index_sequence<indexes...>) consteval
        {
            return std::tuple_cat(element_if_predicate.template operator()<values, indexes>()...);
        }
        (std::make_index_sequence<sizeof...(values)>{});
    }
    template <auto... values>
    using tuple_erase_duplicate_types_t = std::decay_t<decltype(tuple_erase_duplicate_types<values...>())>;
#endif
}

namespace gcl::ctc::tests::algorithms::tuple
{
    namespace ctc_tuple_algorithms = gcl::ctc::algorithms::tuple;
#if not defined(__clang__) and defined(__GNUC__)
    static_assert(
        ctc_tuple_algorithms::tuple_erase_duplicate_values<'a', 'a', 42, 'b', 42, 'a', 13>() ==
        std::tuple{'a', 42, 'b', 13});
    static_assert(std::is_same_v<
                  std::tuple<char, int>,
                  ctc_tuple_algorithms::tuple_erase_duplicate_types_t<'a', 'a', 'b', 42, 'c', 13>>);
#endif
}