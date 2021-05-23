#pragma once

#include <array>
#include <cstddef>
#include <utility>
#include <algorithm>
#include <stdexcept>

namespace gcl::cx
{
    template <typename Key, typename Value, std::size_t N>
    struct map {
        // As-is, quite a naive cx map that deserve more work to become decent

        template <template <typename, typename> typename... Pairs>
        consteval map(Pairs<Key, Value>&&... args)
            : storage{std::forward<decltype(args)>(args)...}
        {}

        using key_type = Key;
        using mapped_type = Value;
        using value_type = std::pair<const key_type, mapped_type>;

        consteval auto& at(const key_type& arg) const
        { // todo : hash
            auto it = std::find_if(
                std::cbegin(storage), std::cend(storage), [&arg](const auto& element) { return element.first == arg; });
            if (it == std::cend(storage))
                throw std::out_of_range{"cx::map::at"};
            return it->second;
        }

      private:
        using storage_type = std::array<value_type, N>;
        const storage_type storage;
    };

    template <typename Key, typename Value, template <typename, typename> typename... Pairs>
    map(Pairs<Key, Value>... args) -> map<Key, Value, sizeof...(Pairs)>;
}

#if defined(GCL_BUILD_CT_TESTS)
namespace gcl::cx::tests
{
    consteval static void map()
    {
        constexpr auto value = cx::map{
            std::pair{'a', 42},
            std::pair{'b', 55},
        };
        static_assert(value.at('a') == 42);
        static_assert(value.at('b') == 55);
    }
}
#endif