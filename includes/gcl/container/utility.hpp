#pragma once

#include <variant>
#include <array>
#include <type_traits>

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

    template <typename... Ts>
    auto generate_array_of_variants(auto&&... args)
    {
        using element_type = std::variant<std::remove_cvref_t<Ts>...>;
        return std::array<element_type, sizeof...(Ts)>{Ts{args...}...};
    }
}

#if defined(GCL_ENABLE_COMPILE_TIME_TESTS)
namespace gcl::container::test
{
    static_assert(std::is_same_v<
                  decltype(make_container_of_variants<std::array>(42, 'a')),
                  std::array<std::variant<int, char>, 2>>);
    static_assert(make_container_of_variants<std::array>(42, 'a') == std::array<std::variant<int, char>, 2>{42, 'a'});
}
#endif