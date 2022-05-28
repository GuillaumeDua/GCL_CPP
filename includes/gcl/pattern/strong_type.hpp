#pragma once 

#include <concepts>

namespace gcl::pattern
{
	template <typename T, typename /*type_tag*/>
    struct strong_type
    {
        using underlying_type = T;
        using reference = T&;
        using const_reference = const T &;

        explicit strong_type(const_reference arg)
        requires std::copy_constructible<T> // is_trivially_copy_constructible_v ?
        : value(arg)
        {}
        explicit strong_type(T&& arg)
        requires std::move_constructible<T>
        : value{ std::forward<decltype(arg)>(arg) }
        {}

        reference       underlying()        { return value; }
        const_reference underlying() const  { return value; }

        operator reference ()               { return underlying(); }
        operator const_reference () const   { return underlying(); }

    private:
        T value;
    };
}