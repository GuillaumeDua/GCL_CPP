#pragma once

#include <gcl/mp/type_traits.hpp>
#include <type_traits>

#if not defined(__clang__)

namespace gcl::mp::type_traits::transpose_qualifier
{
    template <typename from_type, typename to_type>
    using const_t = std::conditional_t<
        std::is_const_v<std::remove_reference_t<from_type>>,
        std::add_const_t<to_type>,
        std::type_identity_t<to_type>>;
    template <typename from_type, typename to_type>
    using volatile_t = std::conditional_t<
        std::is_volatile_v<std::remove_reference_t<from_type>>,
        std::add_volatile_t<to_type>,
        std::type_identity_t<to_type>>;
    template <typename from_type, typename to_type>
    using lvalue_ref_t = std::conditional_t<
        std::is_lvalue_reference_v<from_type>,
        std::add_lvalue_reference_t<to_type>,
        std::type_identity_t<to_type>>;
    template <typename from_type, typename to_type>
    using rvalue_ref_t = std::conditional_t<
        std::is_rvalue_reference_v<from_type>,
        std::add_rvalue_reference_t<to_type>,
        std::type_identity_t<to_type>>;
    template <typename from_type, typename to_type>
    using cvref_t = typename gcl::mp::type_traits::merge_traits_t<
        gcl::mp::type_traits::partial<const_t, from_type>::template type,
        gcl::mp::type_traits::partial<volatile_t, from_type>::template type,
        gcl::mp::type_traits::partial<lvalue_ref_t, from_type>::template type,
        gcl::mp::type_traits::partial<rvalue_ref_t, from_type>::template type>::template type<to_type>;
}
namespace gcl::mp::cast
{
    template <typename to_type>
    decltype(auto) static_cast_preserving_cvref(auto&& value)
    {
        using transpose_cv_ref_t = gcl::mp::type_traits::transpose_qualifier::cvref_t<decltype(value), to_type>;
        return static_cast<transpose_cv_ref_t>(value);
    }
}

#if defined(GCL_ENABLE_COMPILE_TIME_TESTS)
namespace gcl::mp::type_traits::tests::static_cast_preserving_cvref
{
    static_assert(std::is_same_v<
        gcl::mp::type_traits::transpose_qualifier::cvref_t<const int &, char>,
        const char &
    >);
    static_assert(std::is_same_v<gcl::mp::type_traits::transpose_qualifier::cvref_t<volatile int&&, char>, volatile char&&>);
}
namespace gcl::mp::cast::tests::static_cast_preserving_cvref
{
    static_assert(std::is_same_v<
        gcl::mp::type_traits::transpose_qualifier::cvref_t<const char &, int>,
        const int &
    >);
}
#endif
#endif