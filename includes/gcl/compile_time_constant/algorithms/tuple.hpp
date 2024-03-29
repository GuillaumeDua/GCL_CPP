#pragma once

#include <gcl/concepts.hpp>

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
        [[maybe_unused]] constexpr auto element_if_predicate = [predicate]<auto argument>() consteval
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

    template <typename TupleType>
        requires(std::tuple_size_v<TupleType> not_eq 0)
    constexpr auto tuple_to_std_array(TupleType tuple_value)
    {
        return [&tuple_value]<std::size_t... indexes>(std::index_sequence<indexes...>)
        {
            static_assert(gcl::concepts::have_common_type<decltype(std::get<indexes>(tuple_value))...>);
            using element_type = std::common_type_t<decltype(std::get<indexes>(tuple_value))...>;
            return std::array<element_type, sizeof...(indexes)>{std::get<indexes>(tuple_value)...};
        }
        (std::make_index_sequence<std::tuple_size_v<TupleType>>{});
    }

    template <std::size_t N, template <typename...> typename TupleType, typename... TupleElementsTypes>
    constexpr auto tuple_remove_suffix(TupleType<TupleElementsTypes...> tuple_value)
    {
        using tuple_t = TupleType<TupleElementsTypes...>;
        static_assert(std::tuple_size_v<tuple_t> >= N);
        return [&tuple_value]<std::size_t... indexes>(std::index_sequence<indexes...>)
        {
            return TupleType{std::get<indexes>(tuple_value)...};
        }
        (std::make_index_sequence<N>{});
    }
    template <std::size_t N, typename ElementType, auto size>
        requires(size >= N)
    constexpr auto tuple_remove_suffix(std::array<ElementType, size> tuple_value)
    {
        return [&tuple_value]<std::size_t... indexes>(std::index_sequence<indexes...>)
        {
            return std::array{std::get<indexes>(tuple_value)...};
        }
        (std::make_index_sequence<N>{});
    }

    template <std::size_t N, typename TupleType>
        requires(std::tuple_size_v<TupleType> >= N)
    constexpr auto tuple_remove_prefix(TupleType tuple_value)
    {
        return [&tuple_value]<std::size_t... indexes>(std::index_sequence<indexes...>)
        {
            return std::tuple{std::get<N + indexes>(tuple_value)...};
        }
        (std::make_index_sequence<std::tuple_size_v<TupleType> - N>{});
    }
    template <std::size_t N, typename ElementType, auto size>
        requires(size >= N)
    constexpr auto tuple_remove_prefix(std::array<ElementType, size> tuple_value)
    {
        return [&tuple_value]<std::size_t... indexes>(std::index_sequence<indexes...>)
        {
            return std::array<ElementType, size - N>{std::get<N + indexes>(tuple_value)...};
        }
        (std::make_index_sequence<size - N>{});
    }
}

#if defined(GCL_ENABLE_COMPILE_TIME_TESTS)
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

    static_assert(ctc_tuple_algorithms::tuple_to_std_array(std::tuple{'a', 98, 'c'}) == std::array<int, 3>{'a', 'b', 'c'});

    static_assert(ctc_tuple_algorithms::tuple_remove_suffix<2>(std::tuple{'a', 42, 'b', 43}) == std::tuple{'a', 42});
    static_assert(ctc_tuple_algorithms::tuple_remove_suffix<2>(std::array{'a', 'b', 'c'}) == std::array{'a', 'b'});

    static_assert(ctc_tuple_algorithms::tuple_remove_prefix<2>(std::tuple{'a', 42, 'b', 43}) == std::tuple{'b', 43});
    static_assert(ctc_tuple_algorithms::tuple_remove_prefix<2>(std::array{'a', 'b', 'c'}) == std::array{'c'});
}
#endif