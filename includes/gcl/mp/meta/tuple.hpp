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

        template <auto ts_size = sizeof...(types)>
        // enable default constructor that does not collide with value constructor,
        // and cannot be by-passed
        requires(ts_size == sizeof...(types) and size not_eq 0)
        constexpr tuple()
            : storage{generate_storage(types{}...)}
        {}
        template <auto ts_size = sizeof...(types)>
        requires(ts_size == sizeof...(types))
        constexpr tuple(types&&... values)
            : storage{generate_storage(std::forward<decltype(values)>(values)...)}
        {}
        constexpr tuple(tuple&&) = default;
        
      private:
        // storage : by-pass missing features : auto non-static members, lambdas in unevaluated context
        static constexpr auto generate_storage(types&&... values)
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
        };
        decltype(generate_storage(types{}...)) storage;

        template <std::size_t index>
        struct type_at_impl { // defer symbol (Clang)
            using type = std::remove_reference_t<decltype(std::declval<tuple>().template get<index>())>;
        };

      public:
        template <std::size_t index>
        using type_at = typename type_at_impl<index>::type;

        template <std::size_t index>
        requires(not empty and index <= (size - 1)) constexpr auto& get()
        {
            return storage(type_index<index>{});
        }
        template <std::size_t index>
        requires(not empty and index <= (size - 1)) constexpr const auto& get() const
        {
            return const_cast<tuple*>(this)->get<index>(); // violate constness invariants
        }

        template <typename... arg_types>
        requires(sizeof...(arg_types) == size) constexpr bool operator==(const tuple<arg_types...>& other) const
        {
            return [ this, &other ]<std::size_t... indexes>(std::index_sequence<indexes...>)
            {
                return (((get<indexes>() == other.template get<indexes>()) && ...));
            }
            (std::make_index_sequence<size>{});
        }
    };
}

namespace gcl::mp::tests
{
    using namespace gcl::mp;

    using empty_tuple = tuple<>;
    /*using one_element_tuple = tuple<int>;
    using two_element_tuple = tuple<int, char>;
    */
    static constexpr auto empty_tuple_default_init = empty_tuple{};
    /*
    static constexpr auto one_element_tuple_default_init = one_element_tuple{};
    static constexpr auto one_element_tuple_values_init = one_element_tuple{42};
    static constexpr auto two_element_tuple_default_init = two_element_tuple{};
    static constexpr auto two_element_tuple_values_init = two_element_tuple{42, 'a'};

    static_assert(std::is_same_v<tuple<int, char>, decltype(tuple{42, 'a'})>);

    static_assert(std::is_same_v<two_element_tuple::type_at<0>, int>);
    static_assert(std::is_same_v<two_element_tuple::type_at<1>, char>);
    static_assert(std::is_same_v<decltype(std::declval<two_element_tuple>().get<0>()), int&>);
    static_assert(std::is_same_v<decltype(std::declval<const two_element_tuple&>().get<1>()), const char&>);*/

    
}