#pragma once

#include <gcl/functional.hpp>
#include <gcl/mp/typeinfo.hpp>
#include <gcl/mp/function_traits.hpp>

#include <functional>
#include <istream>
#include <ostream>
#include <type_traits>
#include <map>
#include <utility>

// todo : deserializer{ [](auto){} }.deserialize(input);
// todo : typeid-to-type static mapping
// todo : remove std::function
// todo : (de)serialization policy
//          - binary
//          - json
//          - custom ...
// todo : replace istream/ostrem with contracts
//          operator>> , operator<<
//      or  serialize/write , deserialize/read
//  + switch using detection idiom

namespace gcl::serialization::detect
{
    template <typename T, typename = void>
    struct has_custom_serialization : std::false_type
    {
    };
    template <typename T>
    struct has_custom_serialization<
        T,
        std::void_t<decltype(std::declval<T>().serialize_to(std::declval<std::ostream&>))>
    >
        : std::true_type
    {
    };
    template <typename T>
    static inline constexpr auto has_custom_serialization_v = has_custom_serialization<T>::value;

    template <typename T, typename = void>
    struct has_custom_deserialization : std::false_type
    {
    };
    template <typename T>
    struct has_custom_deserialization<
        T,
        std::void_t<decltype(std::declval<T>().deserialize_from(std::declval<std::istream&>))>> : std::true_type
    {
    };
    template <typename T>
    static inline constexpr auto has_custom_deserialization_v = has_custom_deserialization<T>::value;
}
namespace gcl::serialization
{
    template <typename T>
    concept serializable = requires(T arg)
    {
        {not std::is_pointer_v<T>};
        {std::is_standard_layout_v<T>};
        {std::is_default_constructible_v<T>};
        {
            gcl::mp::typeinfo::hashcode_v<T>
        }
        ->std::convertible_to<std::size_t>;
        // {detect::has_custom_serialization_v<T> or (std::declval<std::ostream&>() << std::declval<T>())};
        // {detect::has_custom_deserialization_v<T> or (std::declval<std::istream&>() >> std::declval<T>())};
    };

    namespace binary_data
    {
        template <typename T>
        static inline auto cast(T&& arg)
        {
            using base_type = std::decay_t<std::remove_reference_t<T>>;
            static_assert(std::is_standard_layout_v<base_type>);
            return static_cast<char*>(static_cast<void*>(&arg));
        }
        template <typename T>
        static inline auto cast(const T& arg)
        {
            using base_type = std::decay_t<std::remove_reference_t<T>>;
            static_assert(std::is_standard_layout_v<base_type>);
            return static_cast<const char*>(static_cast<const void*>(&arg));
        }
        template <typename T>
        static inline auto type_id_v = gcl::mp::typeinfo::template hashcode_v<T>;

        using type_id_t = decltype(type_id_v<int>);
        using size_t = decltype(sizeof(int));
        using underlying_data_t = char*;

        template <serializable T>
        struct wrapper
        {
            using underlying_type = T;

            wrapper(T&& arg)
                : type_id{type_id_v<T>}
                , size{sizeof(T)}
                , data{cast(std::forward<T>(arg))}
            {
            }
            wrapper() = default;

            type_id_t         type_id;
            size_t            size;
            underlying_data_t data;
        };
    }

    namespace p1
    { // Proposal 1 : https://github.com/GuillaumeDua/GCL_CPP/issues/16
        struct serializer
        {
            std::ostream& output;

            template <typename... Ts>
            void serialize(Ts&&... args)
            {
                (serialize_impl<Ts>(std::forward<Ts>(args)), ...);
            }

          private:
            template <typename T>
            requires serializable<T> void serialize_impl(T&& arg)
            {
                const auto wrapper = binary_data::wrapper{std::forward<T>(arg)};
                output.write(binary_data::cast(wrapper.type_id), sizeof(decltype(wrapper.type_id)));

                if constexpr (detect::has_custom_serialization_v<T>)
                    std::forward<T>(arg).serialize_to(output);
                else
                    output.write(wrapper.data, wrapper.size);
            }
        };

        template <typename... Ts>
        struct deserializer
        {
            using value_visitor_type = gcl::functional::overload<Ts...>;

            deserializer(value_visitor_type&& visitor_arg)
                : value_visitor{std::forward<decltype(visitor_arg)>(visitor_arg)}
                , types_router{std::move(overload_dynamic_router{value_visitor}.router)}
            {
            }

            void deserialize(std::istream& input) const
            {
                const auto type_id = [this, &input]() {
                    binary_data::type_id_t value;
                    input.read(binary_data::cast(value), sizeof(decltype(value)));
                    if (not input)
                        throw std::runtime_error{"deserializer<Ts...>::deserialize : cannot extract element infos"};
                    return value;
                }();

                const auto& value_accessor = types_router.at(type_id);
                value_accessor(input);
            }
            void deserialize_all(std::istream& input) const
            {
                binary_data::type_id_t type_id;
                while (input)
                {
                    input.read(binary_data::cast(type_id), sizeof(decltype(type_id)));
                    if (not input)
                        break;
                    const auto& value_accessor = types_router.at(type_id);
                    value_accessor(input);
                }
            }

          private:
            value_visitor_type value_visitor;

            struct overload_dynamic_router
            {
                using router_type = std::map<binary_data::type_id_t, std::function<void(std::istream&)>>;
                router_type router;

                template <typename... Ts>
                overload_dynamic_router(gcl::functional::overload<Ts...>& arg)
                    : router{to_dyn_route<Ts>(arg)...}
                {
                }

              private:
                template <typename functor_type, typename overload_type>
                static auto to_dyn_route(overload_type& overload)
                {
                    // todo : ref or copies only -> remove ref for deduction

                    using operator_parenthesis_type = decltype(&functor_type::operator());
                    using functor_trait = typename gcl::mp::function_traits_t<operator_parenthesis_type>;
                    using arguments = functor_trait::template arguments;

                    static_assert(std::tuple_size_v<arguments> == 1);
                    using argument_0_type = typename functor_trait::template argument<0>;

                    static const router_type::key_type hash_code = binary_data::type_id_v<argument_0_type>;

                    return router_type::value_type{
                        hash_code, [&overload](std::istream& is) {
                            using element_type = argument_0_type;
                            element_type element{};

                            if constexpr (detect::has_custom_deserialization_v<element_type>)
                                element.deserialize_from(is);
                            else
                                is.read(serialization::binary_data::cast(element), sizeof(decltype(element)));

                            if (not is)
                                throw std::runtime_error{
                                    "overload_dynamic_router::router : cannot extract element "
                                    "value"};
                            overload(element); // or just store ?
                        }};
                }
            };
            typename overload_dynamic_router::router_type types_router;
        };
        template <typename... Ts>
        deserializer(std::istream&, gcl::functional::overload<Ts...> &&) -> deserializer<Ts...>;

        // todo :
        // deserializer(std::istream&, gcl::functional::overload<Ts...> &&)
        //  -> deserializer<typename functor_trait::template argument<0>(Ts)...>;
    }
}

#include <sstream>
#include <iostream>

namespace gcl::serialization::p1
{
    static void test()
    {
        std::cout << "Testing : serialization (p1) ...\n";
        std::stringstream ss;
        std::cout << "- serializing ...\n";
        gcl::serialization::p1::serializer{ss}.serialize(42, 'a', true);

        auto deserializer = gcl::serialization::p1::deserializer{gcl::functional::overload{
            [](int i) { std::cout << "int : " << i << '\n'; },
            [](char c) { std::cout << "char : " << c << '\n'; },
            [](bool b) { std::cout << "bool : " << b << '\n'; },
            [](float f) { std::cout << "float : " << f << '\n'; },
        }};

        std::cout << "- deserializing ...\n";
        deserializer.deserialize_all(ss);
    }
}

// --- WIP ---

#include <gcl/mp/tuple_utils.hpp>
#include <variant>
#include <array>
#include <gcl/mp/typeinfo.hpp>

namespace gcl::serialization::p5
{
    //using serializer = p1::serializer;

    //template <typename variant_type>
    //struct deserializer
    //{
    //    using element_type = variant_type;

    //    auto get()
    //    {
    //        const auto type_id = [this, &input]() {
    //            binary_data::type_id_t value;
    //            input.read(binary_data::cast(value), sizeof(decltype(value)));
    //            if (not input)
    //                throw std::runtime_error{"deserializer<Ts...>::deserialize : cannot extract element infos"};
    //            return value;
    //        }();

    //        
    //    }
    //    auto get_all()
    //    {
    //        using storage_type = std::vector<variant_type>;
    //    }
    //};

    //static void test()
    //{
    //    std::cout << "Testing : serialization (p5) ...\n";
    //    std::stringstream ss;
    //    std::cout << "- serializing ...\n";
    //    serializer{ss}.serialize(42, 'a', true);

    //    using element_type = std::variant<int, double, char, float, bool>;

    //    auto deserializer = p5::deserializer<element_type>{};

    //    std::cout << "- deserializing ...\n";
    //    //deserializer.deserialize_all(ss);
    //}

    static void test()
    {
        using type = std::variant<int, bool, float>;
            
        constexpr auto mapping = gcl::mp::typeinfo::to_hashcode_array<type>();

        static_assert(mapping[0] == gcl::mp::typeinfo::hashcode_v<int>);
    }
}
