#pragma once

#include <concepts>
#include <type_traits>

namespace gcl::mp::concepts::traits_adapter
{
    // todo : when the standard allows it,
    //        use `template <template <typename> typename... traits, typename T>` instead,
    //        in order to make it usable in template-restriction expression, not only require clauses

    template <typename T, template <typename> typename... traits>
    concept satisfy_all_of = requires
    {
        {
            std::conjunction<traits<T>...> {}
        }
        ->std::convertible_to<std::true_type>;
    };
    template <typename T, template <typename> typename... traits>
    concept satisfy_any_of = requires
    {
        {
            std::disjunction<traits<T>...> {}
        }
        ->std::convertible_to<std::true_type>;
    };
    template <typename T, template <typename> typename... traits>
    concept satisfy_none_of = requires
    {
        {
            std::conjunction<std::negation<traits<T>>...> {}
        }
        ->std::convertible_to<std::true_type>;
    };
}

namespace gcl::mp::concepts::tests::traits_adapter
{
    static_assert(gcl::mp::concepts::traits_adapter::satisfy_all_of<unsigned, std::is_unsigned>);
    static_assert(gcl::mp::concepts::traits_adapter::satisfy_all_of<unsigned, std::is_unsigned, std::is_scalar>);
    static_assert(gcl::mp::concepts::traits_adapter::satisfy_any_of<unsigned, std::is_unsigned, std::is_pointer>);
    static_assert(gcl::mp::concepts::traits_adapter::satisfy_none_of<unsigned, std::is_rvalue_reference, std::is_pointer>);
}