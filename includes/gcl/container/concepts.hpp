#pragma once

#include <utility>
#include <concepts>
#include <type_traits>
#include <iterator>
#include <utility>

namespace gcl::container::concepts
{
    template <typename T>
    concept resizable = requires
    {
        {std::declval<T>().resize(std::declval<typename T::size_type>())};
    };

    template<class T>
    concept LegacyForwardIterator =
        std::constructible_from<T> and
        std::is_lvalue_reference_v<std::iter_reference_t<T>> and
        std::same_as<
            std::remove_cvref_t<std::iter_reference_t<T>>,
            typename std::indirectly_readable_traits<T>::value_type
        > and
        requires(T value) {
            {  value++ } -> std::convertible_to<const T&>;
            { *value++ } -> std::same_as<std::iter_reference_t<T>>;
        };

}