#pragma once

#include <iostream>
#include <concepts>
#include <utility>

namespace gcl::io::concepts
{
    template <typename T>
    concept ostream_shiftable = requires
    {
        {
            std::declval<std::ostream&>() << std::declval<T>()
        }
        ->std::convertible_to<std::ostream&>;
    };
    template <typename T>
    concept istream_shiftable = requires
    {
        {
            std::declval<std::istream&>() >> std::declval<T>()
        }
        ->std::convertible_to<std::istream&>;
    };
    template <typename T>
    concept serializable = std::default_initializable<T>and std::move_constructible<T>;

    template <typename T>
    concept has_custom_serialize = requires
    {
        {std::declval<T>().serialize_to(std::declval<std::ostream&>())};
    };
    template <typename T>
    concept has_custom_deserialize = requires
    {
        {std::declval<T>().deserialize_from(std::declval<std::istream&>())};
    };
    template <typename T>
    concept custom_serializable = has_custom_serialize<T>and has_custom_deserialize<T>;
}