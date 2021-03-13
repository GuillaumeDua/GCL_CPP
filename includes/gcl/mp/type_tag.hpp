#pragma once

#include <utility>
#include <concepts>

namespace gcl::concepts
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
}
namespace gcl::mp::type_tag
{
    template <concepts::tag_type ... tag_arguments>
    class add_tags
    {
        template <concepts::tag_type tag>
            requires ((std::same_as<tag, tag_arguments> || ...))
        struct add_single_tag
        {
            constexpr static typename tag::value_type get_value(typename tag::value_type)
            {
                return tag::value;
            }
        };

    public:
        class type : add_single_tag<tag_arguments>...
        {
            using add_single_tag<tag_arguments>::get_value...;
        public:
            template <typename T>
            constexpr static T value = get_value(T{});
        };    
    };

    template <concepts::tag_type ... tag_arguments>
    using add_tags_t = typename add_tags<tag_arguments...>::type;
}

namespace gcl::mp::tests::type_tag
{
    using not_tagged = gcl::mp::type_tag::add_tags_t<>;

    using tagged = gcl::mp::type_tag::add_tags_t<std::integral_constant<char, 'a'>, std::integral_constant<int, 42>>;
    static_assert(tagged::value<char> == 'a');
    static_assert(tagged::value<int> == 42);
}