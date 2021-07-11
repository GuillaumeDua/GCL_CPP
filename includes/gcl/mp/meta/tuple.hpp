#pragma once

#include <gcl/mp/meta/functional.hpp>
#include <type_traits>

// todo : filter, apply, for_each, for_each_index, swap, operator=

namespace gcl::mp
{
    template <std::size_t index>
    struct type_index {};

    template <typename... types>
    struct tuple {

        constexpr static auto size = sizeof...(types);
        constexpr static auto empty = size == 0;
        template <std::size_t index>
        constexpr static bool valid_index = not empty and index <= (size - 1);

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
                return [value = init_value](type_index<index>) constexpr mutable noexcept -> auto&& { return value; };
            };
            return gcl::mp::meta::functional::overload{
                generate_storage_entry.template operator()<types, indexes>(std::forward<decltype(values)>(values))...};
        };
        using storage_type = decltype(generate_storage(types{}...));
        mutable storage_type storage;

        template <std::size_t index>
        requires(valid_index<index>)
        struct type_at_impl { // defer symbol (Clang)
            using type = std::remove_reference_t<decltype(std::declval<tuple>().template get<index>())>;
        };

      public:
        template <std::size_t index>
        requires(valid_index<index>)
        using type_at = typename type_at_impl<index>::type;

        template <typename T>
        constexpr static bool contains = ((std::size_t{std::is_same_v<T, types>} + ...)) == 1;

        #if defined(_MSC_VER) and not defined(__clang__)
        #pragma message("gcl::mp::tuple::index_of : disabled on msvc-cl")
        #else
        template <typename T>
        constexpr static std::size_t index_of = []() constexpr noexcept -> std::size_t
        {
            static_assert(contains<T>, "only one occurence allowed");
            return []<std::size_t... indexes>(std::index_sequence<indexes...>) constexpr
            {
                return ((std::is_same_v<T, types> ? indexes : 0) + ...);
            }
            (std::make_index_sequence<size>{});
        }
        ();
        #endif

        template <std::size_t index>
        requires(valid_index<index>)
        constexpr auto& get() & noexcept
        {
            return storage(type_index<index>{});
        }
        template <std::size_t index>
        requires(valid_index<index>) [[nodiscard]] constexpr const auto& get() const& noexcept
        {
            return storage(type_index<index>{});
        }
        template <std::size_t index>
        requires(valid_index<index>) [[nodiscard]] constexpr auto&& get() && noexcept
        {
            return std::move(storage(type_index<index>{}));
        }
        template <std::size_t index>
        requires(valid_index<index>) [[nodiscard]] constexpr const auto&& get() const&& noexcept
        {
            return std::move(storage(type_index<index>{}));
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
    [[nodiscard]] constexpr auto& get(tuple<Types...>& value) noexcept
    {
        return value.template get<I>();
    }
    template <std::size_t I, class... Types>
    [[nodiscard]] constexpr auto&& get(tuple<Types...>&& value) noexcept
    {
        return std::move(value.template get<I>());
    }
    template <std::size_t I, class... Types>
    [[nodiscard]] constexpr auto const& get(const tuple<Types...>& value) noexcept
    {
        return value.template get<I>();
    }
    template <std::size_t I, class... Types>
    [[nodiscard]] constexpr auto const&& get(const tuple<Types...>&& value) noexcept
    {
        return std::move(value.template get<I>());
    }

    template <class tuple_type>
    struct tuple_size;
    template <class... Types>
    struct tuple_size<tuple<Types...>> : std::integral_constant<std::size_t, sizeof...(Types)> {};
    template <class T>
    constexpr auto tuple_size_v = tuple_size<T>::value;

    // how to implement this without recursion ?
    template <std::size_t index, class tuple_type>
    struct tuple_element;
    template <std::size_t index, class first, class... rest>
    struct tuple_element<index, tuple<first, rest...>> : tuple_element<(index - 1), tuple<rest...>> {};
    template <class first, class... rest>
    struct tuple_element<0, tuple<first, rest...>> {
        using type = first;        
    };
    template <size_t index>
    struct tuple_element<index, tuple<>> {
        static_assert([]() constexpr { return false; }(), "gcl::mp::tuple : out of range");
    };
    template <std::size_t index, class tuple_type>
    using tuple_element_t = typename tuple_element<index, tuple_type>::type;

    // https://quick-bench.com/q/va85-AzD0ppFuVO6zUTzrC1B6bE
    template <class T, class... Types>
    constexpr T& get(tuple<Types...>& value) noexcept;
    template <class T, class... Types>
    constexpr T&& get(tuple<Types...>&& value) noexcept;
    template <class T, class... Types>
    constexpr const T& get(const tuple<Types...>& value) noexcept;
    template <class T, class... Types>
    constexpr const T&& get(const tuple<Types...>&& value) noexcept;
}

#if defined(GCL_ENABLE_COMPILE_TIME_TESTS)
#include <stdexcept>
#include <exception>
namespace gcl::mp::tests::tuples::values
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

    static_assert(std::is_same_v<decltype(std::declval<two_element_tuple&>().get<0>()), int&>);
    static_assert(std::is_same_v<decltype(std::declval<const two_element_tuple&>().get<1>()), const char&>);
    static_assert(std::is_same_v<decltype(std::declval<two_element_tuple>().get<0>()), int&&>);
    static_assert(std::is_same_v<decltype(std::declval<two_element_tuple&&>().get<0>()), int&&>);

    void non_literal_type(){
        struct can_throw_constructor {
            can_throw_constructor() { throw std::runtime_error{""}; };
        };
        using faillible_tuple = tuple<can_throw_constructor>;
        [[maybe_unused]] const auto faillible_tuple_default_init = faillible_tuple{};
        [[maybe_unused]] const auto faillible_tuple_value_init = faillible_tuple{can_throw_constructor{}};
    }
}
namespace gcl::mp::tests::tuples::get
{
    using namespace gcl::mp;
    using tuple_type = tuple<int, char>;

    static_assert(std::is_same_v<char&, decltype(std::declval<tuple_type&>().get<1>())>);
    static_assert(std::is_same_v<char&&, decltype(std::declval<tuple_type&&>().get<1>())>);
    static_assert(std::is_same_v<const char&, decltype(std::declval<const tuple_type&>().get<1>())>);
    static_assert(std::is_same_v<const char&&, decltype(std::declval<const tuple_type&&>().get<1>())>);

    static_assert(std::is_same_v<char&, decltype(gcl::mp::get<1>(std::declval<tuple_type&>()))>);
    static_assert(std::is_same_v<char&&, decltype(gcl::mp::get<1>(std::declval<tuple_type&&>()))>);
    static_assert(std::is_same_v<const char&, decltype(gcl::mp::get<1>(std::declval<const tuple_type&>()))>);
    static_assert(std::is_same_v<const char&&, decltype(gcl::mp::get<1>(std::declval<const tuple_type&&>()))>);
}
namespace gcl::mp::tests::tuples::size
{
    using namespace gcl::mp;
    using tuple_type = tuple<int, char>;

    static_assert(tuple<>::size == 0);
    static_assert(tuple<>::size == tuple_size_v<tuple<>>);
    static_assert(tuple_type::size == 2);
    static_assert(tuple_type::size == tuple_size_v<tuple_type>);
}
namespace gcl::mp::tests::tuples::contains
{
    using namespace gcl::mp;
    using tuple_type = tuple<int, char>;

    static_assert(tuple_type::contains<int>);
    static_assert(tuple_type::contains<char>);
    static_assert(not tuple_type::contains<double>);
}
#if defined(_MSC_VER) and not defined(__clang__)
#else
namespace gcl::mp::tests::tuples::index_of
{
    using namespace gcl::mp;
    using tuple_type = tuple<int, char>;

    static_assert(tuple_type::index_of<int> == 0);
    static_assert(tuple_type::index_of<char> == 1);
}
#endif
namespace gcl::mp::tests::tuples::tuple_element
{
    using namespace gcl::mp;
    using tuple_type = tuple<int, char>;

    static_assert(std::is_same_v<tuple_element_t<0, tuple_type>, int>);
    static_assert(std::is_same_v<tuple_element_t<1, tuple_type>, char>);
}
#endif