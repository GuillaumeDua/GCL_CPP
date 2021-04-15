#pragma once

#include <utility>
#include <type_traits>
#include <tuple>
#include <cstddef>

namespace gcl::mp::utility
{
	template <typename T, T value>
    constexpr inline auto reverse_integer_sequence_v = []<typename U, U ... indexes>(std::integer_sequence<U, indexes...>)
    {
        return std::integer_sequence<U, (value - 1 - indexes)...>{};
    }(std::make_integer_sequence<T, value>{});

    template <typename T, T value> // std::make_integer_sequence does not use auto-value
    using make_reverse_integer_sequence = std::decay_t<decltype(reverse_integer_sequence_v<T, value>)>;
    template <std::size_t N>
    using make_reverse_index_sequence = make_reverse_integer_sequence<std::size_t, N>;
    template <class ... Ts>
    using reverse_index_sequence_for = make_reverse_index_sequence<sizeof...(Ts)>;

    template <std::size_t ... Ints>
    constexpr inline auto reverse_index_sequence_v = []<std::size_t ... indexes>(std::index_sequence<indexes...>){
        constexpr auto Ints_as_tuple = std::tuple{Ints...};
        return std::index_sequence<std::get<indexes>(Ints_as_tuple)...>{};
    }(reverse_index_sequence_for<decltype(Ints)...>{});

    template <std::size_t ... Ints>
    using reverse_index_sequence = std::decay_t<decltype(reverse_index_sequence_v<Ints...>)>;
}

namespace gcl::mp::utility::tests
{
    static_assert(std::is_same_v<
        make_reverse_index_sequence<3>,
        std::index_sequence<2, 1, 0>>
    );
    static_assert(std::is_same_v<
        reverse_index_sequence<11, 22, 33>,
        std::index_sequence<33, 22, 11>
    >);
}