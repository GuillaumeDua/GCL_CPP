#pragma once

#include <gcl/mp/pack_traits.hpp>
#include <tuple>

namespace gcl::container::details
{
    template <typename T>
    concept TupleType = requires {
        std::tuple_size_v<std::remove_reference_t<T>>;
    };
    template <typename T>
    concept StorageType = TupleType<T> and gcl::mp::type_traits::has_unique_ttps_v<T>;
}
namespace gcl::container
{
    template <typename... Ts>
    requires gcl::mp::type_traits::are_unique_ttps_v<Ts...> using tuple_view_type =
        std::tuple<std::add_lvalue_reference_t<Ts>...>;
    template <typename... Ts>
    requires gcl::mp::type_traits::are_unique_ttps_v<Ts...> using tuple_const_view_type =
        std::tuple<std::add_lvalue_reference_t<std::add_const_t<Ts>>...>;

    template <typename... components_ts>
    constexpr auto make_tuple_view(details::StorageType auto& storage)
    requires requires {
        ((std::get<components_ts>(storage)), ...);
    }
    {
        using result_type = tuple_view_type<components_ts...>;
        return result_type{std::get<components_ts>(storage)...};
    }
    template <typename... components_ts>
    constexpr auto make_tuple_view(const details::StorageType auto& storage)
    requires requires {
        ((std::get<components_ts>(storage)), ...);
    }
    {
        using result_type = tuple_const_view_type<components_ts...>;
        return result_type{std::get<components_ts>(storage)...};
    }
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
    consteval void tuple_view_ref()
    {
        auto features_available = std::tuple{42, 'A', double{13}};
        auto process_dispatcher = [](auto& value, auto& features_container) constexpr {
            auto features_requested = [&features_container]<template <typename...> typename T, typename... Ts>(T<Ts...>)
            {
                return gcl::container::make_tuple_view<Ts...>(features_container);
            }
            (typename std::remove_cvref_t<decltype(value)>::signature_type{});
            value.process(std::move(features_requested));
        };

        feature<int, char> c1;
        feature<int>       c2;
        feature<char>      c3;

        process_dispatcher(c1, features_available); // 43, B
        process_dispatcher(c2, features_available); // 44
        process_dispatcher(c3, features_available); // C
    }
}
#endif
