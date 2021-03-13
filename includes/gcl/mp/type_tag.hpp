#pragma once

#include <utility>
#include <concepts>

namespace gcl::mp::type_tag
{
    template <typename tag_t>
    concept tag_type = requires(tag_t)
    {
        typename tag_t::value_type;
        {
            std::decay_t<decltype(tag_t::value)> {}
        }
        ->std::same_as<typename tag_t::value_type>;
    };

    template <tag_type tag>
    struct add_tag {
        constexpr static typename tag::value_type get_value(typename tag::value_type) { return tag::value; }
    };

    template <tag_type... tag_arguments>
    class add_tags : add_tag<tag_arguments>... {
        using add_tag<tag_arguments>::get_value...;

      public:
        template <typename T>
        constexpr static T value = get_value(T{});
    };
}

namespace gcl::mp::tests::type_tag
{
    using tagged = gcl::mp::type_tag::add_tags<std::integral_constant<char, 'a'>, std::integral_constant<int, 42>>;
    static_assert(tagged::value<char> == 'a');
    static_assert(tagged::value<int> == 42);
}