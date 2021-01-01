#pragma once

namespace gcl::functional
{
    template <class... Ts>
    struct overload : Ts...
    {
        using Ts::operator()...;
    };
    template <class... Ts>
    overload(Ts...) -> overload<Ts...>;

    template <typename T>
    decltype(auto) wrap(T && func)
    {   // wrap any functor into a deductible type/value
        return [func](auto &&... args) -> decltype(auto) { return std::invoke(func, std::forward<decltype(args)>(args)...); };
    }
}
