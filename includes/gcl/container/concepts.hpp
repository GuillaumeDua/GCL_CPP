#pragma once

#include <utility>

namespace gcl::container::concepts
{
    template <typename T>
    concept resizable = requires
    {
        {std::declval<T>().resize(std::declval<typename T::size_type>())};
    };
}