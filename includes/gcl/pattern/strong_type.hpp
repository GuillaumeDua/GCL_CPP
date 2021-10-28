#pragma once 

#include <type_traits>

namespace gcl::pattern
{
	template <typename T, typename /*type_tag*/>
    struct strong_type
    {
        using underlying_type = T;
        using reference = T&;
        using const_reference = const T &;

        explicit strong_type(const_reference arg)
        requires std::is_copy_constructible_v<T> // is_trivially_copy_constructible_v ?
        : value(arg)
        {}
        explicit strong_type(T&& arg)
        requires std::is_move_constructible_v<T>
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