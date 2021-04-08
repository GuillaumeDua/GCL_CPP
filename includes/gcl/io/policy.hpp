#pragma once

#include <gcl/container/concepts.hpp>
#include <gcl/io/concepts.hpp>
#include <gcl/mp/concepts.hpp>

#include <concepts>
#include <utility>
#include <type_traits>

#include <iostream>

namespace gcl::io::policy
{
    template <typename policy_impl>
    struct crtp_impl {
        template <typename T>
        // requires gcl::io::concepts::serializable<std::remove_reference_t<T>> // todo
        static void read(std::istream& is, T&& value)
        {
            policy_impl::basic_read(is, std::forward<decltype(value)>(value));
        }
        template <gcl::io::concepts::has_custom_deserialize T>
        static void read(std::istream& is, T&& value)
        { // policy by-pass
            value.deserialize_from(is);
        }
        template <gcl::concepts::pointer T> // [modif]
        static void read(std::istream& is, T&& value)
        {
            policy_impl::read(is, *value);
        }
        template <std::ranges::range T>
        static void read(std::istream& is, T&& value)
        {
            using value_type = std::remove_reference_t<T>;
            const auto size = [&is]() {
                if constexpr (std::is_array_v<value_type>)
                {
                    std::size_t size_value;
                    policy_impl::read(is, size_value);
                    return size_value;
                }
                else
                {
                    typename value_type::size_type size_value;
                    policy_impl::read(is, size_value);
                    return size_value;
                }
            }();

            if constexpr (gcl::container::concepts::resizable<value_type>)
                value.resize(size);
            else if (std::size(value) < size)
                throw std::runtime_error{"gcl::io::policy::policy_impl::read : fixed-size < size"};

            auto element = []() {
                if constexpr (std::is_array_v<value_type>)
                    return std::remove_extent_t<value_type>{};
                else
                    return typename value_type::value_type{};
            }(); // Clang does not support lambdas in an unevaluated context yet ...

            auto input_it = std::begin(value);
            for (std::decay_t<decltype(size)> i{0}; i < size; ++i)
            {
                policy_impl::read(is, element);
                *input_it++ = std::move(element);
            }
        }
        template <typename... Ts, typename = std::enable_if_t<(sizeof...(Ts) not_eq 1)>>
        static void read(std::istream& is, Ts&&... values)
        {
            (read(is, std::forward<Ts>(values)), ...);
        }

        template <typename T>
        static void write(std::ostream& os, const T& value)
        {
            if constexpr (std::is_pointer_v<T>)
                write(os, *value);
            else if constexpr (std::ranges::range<T>)
            {
                write(os, std::size(value));
                for (const auto& element : value)
                    write(os, element);
            }
            else
                policy_impl::basic_write(os, value);
        }
        template <gcl::io::concepts::has_custom_serialize T>
        static void write(std::ostream& os, const T& value)
        { // policy by-pass
            value.serialize_to(os);
        }
        template <typename... Ts, typename = std::enable_if_t<(sizeof...(Ts) not_eq 1)>>
        static void write(std::ostream& os, const Ts&... values)
        {
            (write(os, values), ...);
        }
    };

    struct binary : crtp_impl<binary> {
        using crtp_impl<binary>::read;
        using crtp_impl<binary>::write;

        template <typename T>
        static void basic_read(std::istream& is, T&& value)
        {
            is.read(reinterpret_cast<char*>(&value), sizeof(T));
        }
        template <typename T>
        static void basic_write(std::ostream& os, const T& value)
        {
            os.write(reinterpret_cast<const char*>(&value), sizeof(T));
        }
    };
    template <>
    void binary::basic_read<std::string>(std::istream& is, std::string&& value)
    {
        std::string::size_type size;
        binary::basic_read(is, size);
        value.resize(size, '\0');
        is.read(&value[0], size);
    }
    template <>
    void binary::basic_write<std::string>(std::ostream& os, const std::string& value)
    {
        std::string::size_type size = value.length();
        os.write(reinterpret_cast<char*>(&size), sizeof(std::string::size_type));
        os.write(&value[0], value.length());
    }

    struct stream : crtp_impl<stream> {
        using crtp_impl<stream>::read;
        using crtp_impl<stream>::write;

        constexpr static char record_separator = 0x1e; // RS ASCII entry

        template <typename T>
            requires concepts::has_custom_deserialize<T> or
            concepts::istream_shiftable<T> static void basic_read(std::istream& is, T&& value)
        {
            // todo : if constexpr has_custom_deserialize
            //    deserialize
            // else
            extract_RS(is >> std::forward<T>(value));
        }
        template <typename T>
            requires concepts::has_custom_serialize<T> or
            concepts::ostream_shiftable<T> static void basic_write(std::ostream& os, const T& value)
        {
            os << value << record_separator;
        }

      private:
        static void extract_RS(std::istream& is)
        {
            decltype(record_separator) separator = is.get();
            if (separator not_eq record_separator)
                throw std::runtime_error{"gcl::io::policy::stream : unexpected RS"};
        }
    };

    static_assert(sizeof(binary{}) == 1);
    static_assert(sizeof(stream{}) == 1);
}