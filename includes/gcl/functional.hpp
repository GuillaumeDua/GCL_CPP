#pragma once

#include <utility>

namespace gcl::functional
{
    template <class... Ts>
    struct overload : Ts... {
        using Ts::operator()...;
        using bases_types = std::tuple<Ts...>; // allow operator() function_traits
    };

    template <typename T>
    decltype(auto) wrap(T&& func)
    { // wrap any functor into a deductible type/value
        return [func](auto&&... args) -> decltype(auto) {
            return std::invoke(func, std::forward<decltype(args)>(args)...);
        };
    }
}
