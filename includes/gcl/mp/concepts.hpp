#pragma once

#include <concepts>
#include <type_traits>

namespace gcl::mp::concepts::traits_adapter
{
    // todo : when the standard allows it,
    //        use `template <template <typename> typename... traits, typename T>` instead,
    //        in order to make it usable in template-restriction expression, not only require clauses

    // [04/15/2021] :   Because GCC 10.2 is the current Ubuntu 20.04 LTS latest GCC release,
    //                  and it does not support most c++ `concept` synthaxes (and stdlibc++ concept library
    //                  implementation is incomplete, the current implementation uses `{ trait{} } ->
    //                  std::derived_from<std::true_type>` as equality check
    // see https://godbolt.org/z/MsY4P1arj for details
#if not defined(__GNUC_PREREQ)
#define __GNUC_PREREQ(major, minor) false
#endif
#if (not defined(__clang__) and defined(__GNUC__) and not __GNUC_PREREQ(10, 3))
#pragma message(                                                                                                       \
    "[gcl::mp::concepts::traits_adapter] switching to compiler-specific implementation details because GCC " __VERSION__)

    template <typename T, template <typename> typename... traits>
    concept satisfy_all_of = requires
    {
        {
            std::conjunction<traits<T>...> {}
        }
        ->std::derived_from<std::true_type>;
    };
    template <typename T, template <typename> typename... traits>
    concept satisfy_any_of = requires
    {
        {
            std::disjunction<traits<T>...> {}
        }
        ->std::derived_from<std::true_type>;
    };
    template <typename T, template <typename> typename... traits>
    concept satisfy_none_of = requires
    {
        {
            std::conjunction<std::negation<traits<T>>...> {}
        }
        ->std::derived_from<std::true_type>;
    };
#else
    template <typename T, template <typename> typename... traits>
    concept satisfy_all_of = requires
    {
        requires(std::conjunction_v<traits<T>...>);
    };
    template <typename T, template <typename> typename... traits>
    concept satisfy_any_of = requires
    {
        requires(std::disjunction_v<traits<T>...>);
    };
    template <typename T, template <typename> typename... traits>
    concept satisfy_none_of = requires
    {
        requires(std::conjunction_v<std::negation<traits<T>>...>);
    };
#endif
}

namespace gcl::mp::concepts::tests::traits_adapter
{
    static_assert(std::is_unsigned<unsigned>::value);
    static_assert(std::conjunction_v<std::is_unsigned<unsigned>>);
    static_assert(gcl::mp::concepts::traits_adapter::satisfy_all_of<unsigned, std::is_unsigned>);
    static_assert(gcl::mp::concepts::traits_adapter::satisfy_all_of<unsigned, std::is_unsigned, std::is_scalar>);
    static_assert(gcl::mp::concepts::traits_adapter::satisfy_any_of<unsigned, std::is_unsigned, std::is_pointer>);
    static_assert(gcl::mp::concepts::traits_adapter::satisfy_none_of<unsigned, std::is_rvalue_reference, std::is_pointer>);
}