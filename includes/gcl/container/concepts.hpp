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
        std::constructible_from<T>
        and std::is_lvalue_reference_v<std::iter_reference_t<T>>
        and std::same_as<
            std::remove_cvref_t<std::iter_reference_t<T>>,
            typename std::indirectly_readable_traits<T>::value_type
        >
        and requires(T value) {
            {  value++ } -> std::convertible_to<const T&>;
            { *value++ } -> std::same_as<std::iter_reference_t<T>>;
        };

    template <typename T>
    concept Container =
        std::regular<T>
        and std::swappable<T>
        and requires (T value) { // very naive container requirements
            typename T::value_type;
            typename T::reference;
            typename T::const_reference;
            typename T::iterator;
            typename T::const_iterator;
            typename T::difference_type;
            typename T::size_type;

            { std::begin(value) } -> std::same_as<typename T::iterator>;
            { std::end(value) } -> std::same_as<typename T::iterator>;
            { std::cbegin(value) } -> std::same_as<typename T::const_iterator>;
            { std::cend(value) } -> std::same_as<typename T::const_iterator>;

            { std::size(value) } -> std::same_as<typename T::size_type>;
            { std::empty(value) } -> std::convertible_to<bool>;
        }
        and LegacyForwardIterator<typename T::iterator>
        and LegacyForwardIterator<typename T::const_iterator>
        and std::convertible_to<typename T::iterator, typename T::const_iterator>
        and std::same_as<
            typename T::difference_type,
            typename std::iterator_traits<typename T::iterator>::difference_type
        >
        and std::same_as<
            typename T::difference_type,
            typename std::iterator_traits<typename T::const_iterator>::difference_type
        >
    ;
}
