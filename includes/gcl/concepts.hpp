#pragma once

#include <gcl/mp/type_traits.hpp>
#include <concepts>
#include <type_traits>

namespace gcl::concepts
{
    template <typename T>
    concept pointer = (std::is_pointer<T>::value);

    template <typename T>
    concept function =
        std::is_function_v<T> or std::is_function_v<std::remove_pointer_t<T>> or std::is_member_function_pointer_v<T>;

    template <typename... Ts>
    concept have_common_type = requires()
    {
        std::common_type_t<Ts...>{};
    };

    template <typename T>
    concept std_array = gcl::mp::type_traits::template is_std_array_v<T>;
    template <typename T>
    concept raw_array = std::is_array_v<T>;
    template <typename T>
    concept array_ = std_array<T> or raw_array<T>; // trailing '_', as `array` is reserved keyword
}

namespace gcl::concepts::tests
{
    static_assert(gcl::concepts::pointer<int*>);
    static_assert(not gcl::concepts::pointer<int>);

    void func() {}
    struct A {
        void        func(){};
        void        const_func() const {};
        static void static_func(){};
    };
    constexpr void function()
    {
        static_assert(gcl::concepts::function<decltype(&func)>);
        static_assert(gcl::concepts::function<decltype(&A::func)>);
        static_assert(gcl::concepts::function<decltype(&A::const_func)>);
        static_assert(gcl::concepts::function<decltype(&A::static_func)>);
    }
    constexpr void arrays()
    {
        using std_array_type = std::array<int, 4>;
        using raw_array_type = char[4];

        static_assert(gcl::concepts::array_<std_array_type> and gcl::concepts::array_<raw_array_type>);
        static_assert(gcl::concepts::std_array<std_array_type>);
        static_assert(not gcl::concepts::std_array<raw_array_type>);
        static_assert(not gcl::concepts::raw_array<std_array_type>);
        static_assert(gcl::concepts::raw_array<raw_array_type>);
    }
}