#pragma once

#include <variant>
#include <array>

namespace gcl::container
{
    template <template <typename> typename container_type, typename... Ts>
    auto make_container_of_variants(Ts&&... values)
    {
        using value_type = std::variant<std::remove_cvref_t<Ts>...>;
        return container_type{value_type{std::forward<decltype(values)>(values)}...};
    }
    template <template <typename, std::size_t> typename container_type, typename... Ts>
    auto make_container_of_variants(Ts&&... values)
    {
        using value_type = std::variant<std::remove_cvref_t<Ts>...>;
        return container_type{value_type{std::forward<decltype(values)>(values)}...};
    }
}

#if defined(GCL_ENABLE_COMPILE_TIME_TESTS)
static_assert(
    std::is_same_v<decltype(make_container_of_variants<std::array>(42, 'a')), std::array<std::variant<int, char>, 2>>);
static_assert(make_container_of_variants<std::array>(42, 'a') == std::array<std::variant<int, char>, 2>{42, 'a'});
#endif