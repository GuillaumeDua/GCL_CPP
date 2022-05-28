#pragma once

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
}

#if defined(GCL_ENABLE_COMPILE_TIME_TESTS)
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
}
#endif