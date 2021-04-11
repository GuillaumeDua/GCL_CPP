#pragma once

#include <utility>
#include <concepts>
#include <type_traits>

namespace gcl::mp::type_tag
{
    template <typename T>
    struct type_to_type {
        using type = T;
    };
    template <typename T>
    using type_to_type_t = typename type_to_type<T>::type;

    template <typename... Ts>
    class container {
        template <typename T>
        static constexpr bool contains_impl = ((std::is_same_v<T, Ts> or ...));

      public:
        template <typename... Tags>
        static constexpr bool contains = ((contains_impl<Tags> and ...));
    };
}

namespace gcl::mp::concepts
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

namespace gcl::mp::tests::type_tag::type_to_type
{
    static_assert(std::is_same_v<gcl::mp::type_tag::type_to_type_t<int>, int>);
    constexpr void test()
    {
        int  i{0};
        auto lambda_with_capture = [&i]() { (void)i; };
        static_assert(std::is_same_v<
                      gcl::mp::type_tag::type_to_type_t<decltype(lambda_with_capture)>,
                      decltype(lambda_with_capture)>);
    }
}
namespace gcl::mp::tests::type_tag::container
{
    struct tag_1 {};
    struct tag_2 {};
    using holder = gcl::mp::type_tag::container<tag_1, tag_2>;

    static_assert(holder::contains<>);
    static_assert(holder::contains<tag_1>);
    static_assert(holder::contains<tag_2>);
    static_assert(holder::contains<tag_1, tag_2>);
    static_assert(not holder::contains<int, tag_1, tag_2>);
}
namespace gcl::mp::tests::type_tag::add_tags
{
    using not_tagged = gcl::mp::type_tag::add_tags_t<>;

    using tagged = gcl::mp::type_tag::add_tags_t<std::integral_constant<char, 'a'>, std::integral_constant<int, 42>>;
    static_assert(tagged::value<char> == 'a');
    static_assert(tagged::value<int> == 42);
}