#pragma once

#include <gcl/container/concepts.hpp>

#include <concepts>
#include <utility>
#include <type_traits>

#include <iostream>

namespace gcl::io::policies
{
    struct binary {
        // out
        template <typename T>
        static void read(std::istream& is, T& value)
        {
            if constexpr (std::is_pointer_v<T>)
                is.read(reinterpret_cast<char*>(value), sizeof(T));
            else if constexpr (std::ranges::range<T>)
            {
                const auto size = [&is]() {
                    if constexpr (std::is_array_v<T>)
                    {
                        std::size_t size_value;
                        read(is, size_value);
                        return size_value;
                    }
                    else
                    {
                        typename T::size_type size_value;
                        read(is, size_value);
                        return size_value;
                    }
                }();

                if constexpr (gcl::container::concepts::resizable<T>)
                    value.resize(size);
                else if (std::size(value) < size)
                    throw std::runtime_error{"gcl::io::policy::binary::read : fixed-size < size"};

                using element_type = decltype([]() {
                    if constexpr (std::is_array_v<T>)
                        return std::remove_extent_t<T>{};
                    else
                        return typename T::value_type{};
                }());
                static_assert(std::is_default_constructible_v<element_type>);
                element_type element;
                auto         input_it = std::begin(value);
                for (std::decay_t<decltype(size)> i{0}; i < size; ++i)
                {
                    read(is, element);
                    *input_it++ = std::move(element);
                }
            }
            else
                is.read(reinterpret_cast<char*>(&value), sizeof(T));
        }
        // in
        template <typename T>
        static void write(std::ostream& os, const T& value)
        {
            if constexpr (std::is_pointer_v<T>)
                os.write(reinterpret_cast<const char*>(value), sizeof(T));
            else if constexpr (std::ranges::range<T>)
            {
                write(os, std::size(value));
                for (const auto& element : value)
                    write(os, element);
            }
            else
                os.write(reinterpret_cast<const char*>(&value), sizeof(T));
        }
    };

    template <>
    void binary::read<std::string>(std::istream& is, std::string& value)
    {
        std::string::size_type size;
        read(is, size);
        value.resize(size, '\0');
        is.read(&value[0], size);
    }
    template <>
    void binary::write<std::string>(std::ostream& os, const std::string& value)
    {
        std::string::size_type size = value.length();
        os.write(reinterpret_cast<char*>(&size), sizeof(std::string::size_type));
        os.write(&value[0], value.length());
    }

    struct stream {
        template <typename T>
        static void write(std::ostream& os, const T& value)
        {
            if constexpr (std::ranges::range<T>)
            {
                static_assert(false, "not implemented yet");
            }
            os << value;
        }
        template <typename T>
        static void read(std::istream& is, T& value)
        {
            if constexpr (std::ranges::range<T>)
            {
                static_assert(false, "not implemented yet");
            }
            is >> value;
        }
        template <typename T>
        static void write(std::ostream& os, const T* value)
        {
            write<T>(os, *value);
        }
        template <typename T>
        static void read(std::istream& is, T* value)
        {
            read<T>(is, *value);
        }
    };
}