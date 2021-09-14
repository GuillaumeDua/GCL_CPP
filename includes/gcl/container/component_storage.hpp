#pragma once

#include <mp/pack_traits.hpp>
#include <tuple>
#include <type_traits.hpp>

namespace gcl::container
{
    template <typename... Ts>
    requires gcl::mp::are_unique_v<Ts...> struct components_storage {
        static_assert(not(std::is_reference_v<Ts> or ...));
        using storage_type = std::tuple<Ts...>;

        constexpr components_storage() requires(std::is_default_constructible_v<Ts>&&...) {}
        constexpr components_storage(Ts&&... args) requires(std::is_move_constructible_v<Ts>&&...)
            : storage{std::forward<decltype(args)>(args)...}
        {}

        template <typename... features_names>
        constexpr auto get()
        {
            static_assert(
                ([]<typename T>() { return (std::is_same_v<T, Ts> or ...); }.template operator()<features_names>() and
                 ...),
                "components_storage : missing requested components");

            return std::tuple<features_names&...>{std::get<features_names>(storage)...};
        }
        template <typename... features_names>
        constexpr auto get() const
        {
            static_assert(
                ([]<typename T>() { return (std::is_same_v<T, Ts> or ...); }.template operator()<features_names>() and
                 ...),
                "components_storage : missing requested components");

            return std::tuple<const features_names&...>{std::get<features_names>(storage)...};
        }

      private:
        storage_type storage;
    };
    template <typename... Ts>
    components_storage() -> components_storage<std::remove_reference_t<Ts>...>;
    template <typename... Ts>
    components_storage(Ts&&...) -> components_storage<std::remove_reference_t<Ts>...>;
}

#if defined(GCL_ENABLE_COMPILE_TIME_TESTS)
namespace gcl::container::test
{
    template <typename... Ts>
    struct feature {

        using signature_type = std::tuple<Ts...>;

        constexpr void process(std::tuple<Ts&...>&& args)
        {
            [&args]<std::size_t... indexes>(std::index_sequence<indexes...>) { ((++std::get<indexes>(args)), ...); }
            (std::make_index_sequence<sizeof...(Ts)>());
        }
    };
    consteval void cs()
    {
        constexpr auto features_available = components_storage<int, char, double>{42, 'A', 13.d};
        constexpr auto process_dispatcher = [](auto& value, auto& features_container) constexpr {
            auto features_requested = [&features_container]<template <typename...> typename T, typename... Ts>(T<Ts...>)
            {
                return features_container.template get<Ts...>();
            }
            (typename std::remove_cvref_t<decltype(value)>::signature_type{});
            value.process(std::move(features_requested));
        };

        feature<int, char> c1;
        feature<int>       c2;
        feature<char>      c3;

        process_dispatcher(c1, features_available);
        static_assert(features_available.get<int>() == 43);
        static_assert(features_available.get<char>() == 'B');
        process_dispatcher(c2, features_available);
        static_assert(features_available.get<int>() == 44);
        static_assert(features_available.get<char>() == 'B');
        process_dispatcher(c3, features_available);
        static_assert(features_available.get<int>() == 43);
        static_assert(features_available.get<char>() == 'C');
    }
}
#endif
