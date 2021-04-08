#pragma once

#include <concepts>
#include <type_traits>

namespace gcl::concepts
{
    template <typename T>
    concept pointer = (std::is_pointer<T>::value);
}

namespace gcl::concepts::tests
{
    static_assert(gcl::concepts::pointer<int*>);
    static_assert(not gcl::concepts::pointer<int>);
}