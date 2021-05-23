#pragma once

#include <gcl/mp/meta/functional.hpp>

namespace gcl::mp
{
    template <std::size_t index>
    struct type_index {};

    template <typename... types>
    struct tuple {

        constexpr static auto size = sizeof...(types);
        constexpr static auto empty = size == 0;

        constexpr tuple() requires(not empty)
            : storage{generate_storage(types{}...)}
        {}
        constexpr tuple(types&&... values)
            : storage{generate_storage(std::forward<decltype(values)>(values)...)}
        {}
        constexpr tuple(tuple&&) = default;

      private:
        // storage : by-pass missing features : auto non-static members, lambdas in unevaluated context
        using index_sequence = std::make_index_sequence<sizeof...(types)>;
        static constexpr auto generate_storage(types&&... values)
        { // defer parameter pack expansion (msvc-cl, Clang)
            return generate_storage_impl(index_sequence{}, std::forward<decltype(values)>(values)...);
        }
        template <std::size_t... indexes>
        static constexpr auto generate_storage_impl(std::index_sequence<indexes...>, types&&... values)
        {
            static_assert(sizeof...(indexes) == sizeof...(types));
            static_assert(sizeof...(indexes) == sizeof...(values));

            const auto generate_storage_entry = []<typename T, std::size_t index>(T && init_value) constexpr
            {
                return [value = init_value](type_index<index>) mutable noexcept -> auto& { return value; };
            };
            return gcl::mp::meta::functional::overload{
                generate_storage_entry.template operator()<types, indexes>(std::forward<decltype(values)>(values))...};
        };
        using storage_type = decltype(generate_storage(types{}...));
        storage_type storage;

        template <std::size_t index>
        struct type_at_impl { // defer symbol (Clang)
            using type = std::remove_reference_t<decltype(std::declval<tuple>().template get<index>())>;
        };

      public:
        template <std::size_t index>
        using type_at = typename type_at_impl<index>::type;

        template <std::size_t index>
        requires(not empty and index <= (size - 1)) constexpr auto& get() noexcept
        {
            return storage(type_index<index>{});
        }
        template <std::size_t index>
        requires(not empty and index <= (size - 1)) constexpr const auto& get() const noexcept
        {
            return const_cast<tuple*>(this)->get<index>(); // somehow violates constness invariants
        }

        template <typename... arg_types>
        requires(sizeof...(arg_types) == size) constexpr bool operator==(
            const tuple<arg_types...>& other) const noexcept
        {
            return [ this, &other ]<std::size_t... indexes>(std::index_sequence<indexes...>)
            {
                return (((get<indexes>() == other.template get<indexes>()) && ...));
            }
            (std::make_index_sequence<size>{});
        }
    };
    // When Clang, Msvc-cl gets better, replace `tuple::generate_storage` implementation by :
    /*static constexpr auto generate_storage(types&&... values)
    {
        return [&values...]<std::size_t... indexes>(std::index_sequence<indexes...>)
        {
            static_assert(sizeof...(indexes) == sizeof...(types));
            static_assert(sizeof...(indexes) == sizeof...(values));

            const auto generate_storage_entry = []<typename T, std::size_t index>(T && init_value) constexpr
            {
                return [value = init_value](type_index<index>) mutable -> auto& { return value; };
            };
            return gcl::mp::meta::functional::overload{generate_storage_entry.template operator()<types, indexes>(
                std::forward<decltype(values)>(values))...};
        }
        (std::make_index_sequence<sizeof...(types)>{});
    };*/
}

namespace gcl::mp
{
    template <std::size_t I, class... Types>
    constexpr auto get(tuple<Types...>& t) noexcept;
    template <std::size_t I, class... Types>
    constexpr auto get(tuple<Types...>&& t) noexcept;
    template <std::size_t I, class... Types>
    constexpr auto const& get(const tuple<Types...>& t) noexcept;
    template <std::size_t I, class... Types>
    constexpr auto const&& get(const tuple<Types...>&& t) noexcept;

    template <class T, class... Types>
    constexpr T& get(tuple<Types...>& t) noexcept;
    template <class T, class... Types>
    constexpr T&& get(tuple<Types...>&& t) noexcept;
    template <class T, class... Types>
    constexpr const T& get(const tuple<Types...>& t) noexcept;
    template <class T, class... Types>
    constexpr const T&& get(const tuple<Types...>&& t) noexcept;
}

#if defined(GCL_ENABLE_COMPILE_TIME_TESTS)
#include <stdexcept>
#include <exception>
namespace gcl::mp::tests::tuples
{
    using namespace gcl::mp;

    using empty_tuple = tuple<>;
    using one_element_tuple = tuple<int>;
    using two_element_tuple = tuple<int, char>;
    
    static constexpr auto empty_tuple_default_init = empty_tuple{};
    static constexpr auto one_element_tuple_default_init = one_element_tuple{};
    static constexpr auto two_element_tuple_default_init = two_element_tuple{};

    static constexpr auto one_element_tuple_values_init = one_element_tuple{42};
    static constexpr auto two_element_tuple_values_init = two_element_tuple{42, 'a'};

    static_assert(std::is_same_v<tuple<int, char>, decltype(tuple{42, 'a'})>);

    static_assert(std::is_same_v<two_element_tuple::type_at<0>, int>);
    static_assert(std::is_same_v<two_element_tuple::type_at<1>, char>);
    static_assert(std::is_same_v<decltype(std::declval<two_element_tuple>().get<0>()), int&>);
    static_assert(std::is_same_v<decltype(std::declval<const two_element_tuple&>().get<1>()), const char&>);


    void non_literal_type(){
        struct can_throw_constructor {
            can_throw_constructor() { throw std::runtime_error{""}; };
        };
        using faillible_tuple = tuple<can_throw_constructor>;
        [[maybe_unused]] const auto faillible_tuple_default_init = faillible_tuple{};
        [[maybe_unused]] const auto faillible_tuple_value_init = faillible_tuple{can_throw_constructor{}};
    }
}
#endif