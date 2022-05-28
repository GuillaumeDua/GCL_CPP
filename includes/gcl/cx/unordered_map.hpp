#pragma once

#include <algorithm>
#include <array>
#include <concepts>
#include <cstddef>
#include <stdexcept>
#include <tuple>
#include <utility>

namespace gcl::cx
{
    template <typename Key, typename Value, std::size_t N>
    struct unordered_map {

        using key_type = Key;
        using mapped_type = Value;
        using type = unordered_map<Key, Value, N>;
        using value_type = std::pair<const key_type, mapped_type>;

        constexpr static auto size = N;

        consteval unordered_map(auto&&... args)
            : storage{value_type{std::get<0>(args), std::get<1>(args)}...}
        {}

        [[nodiscard]] consteval auto& at(const key_type& arg) const
        {
            auto it = std::find_if(
                std::cbegin(storage), std::cend(storage), [&arg](const auto& element) constexpr {
                    return element.first == arg;
                });
            if (it == std::cend(storage))
                throw std::out_of_range{"gcl::cx::unordered_map::at"};
            return it->second;
        }

        constexpr decltype(auto) begin() { return std::begin(storage); }
        constexpr decltype(auto) end() { return std::end(storage); }
        constexpr decltype(auto) begin() const { return std::begin(storage); }
        constexpr decltype(auto) end() const { return std::end(storage); }
        constexpr decltype(auto) cbegin() const { return std::cbegin(storage); }
        constexpr decltype(auto) cend() const { return std::cend(storage); }

      private:
        using storage_type = std::array<value_type, N>;
        const storage_type storage;
    };

    template <typename ... Ts>
    unordered_map(Ts &&... args) -> unordered_map<
        std::common_type_t<decltype(std::get<0>(args))...>,
        std::common_type_t<decltype(std::get<1>(args))...>,
        sizeof...(args)
    >;
}

namespace gcl::cx::tests
{
    consteval static void map()
    {
        constexpr auto value = cx::unordered_map{
            std::pair{'a', 42},
            std::tuple{'b', 55},
        };
        static_assert(value.at('a') == 42);
        static_assert(value.at('b') == 55);

        for (const auto& [k, v] : value)
        {}
    }
}