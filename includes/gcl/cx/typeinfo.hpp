#pragma once

#include <gcl/cx/crc32_hash.hpp>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

namespace gcl::cx::typeinfo::details
{
    struct type_prefix_tag {
        constexpr static std::string_view value = "T = ";
    };
    struct value_prefix_tag {
        constexpr static std::string_view value = "value = ";
    };

    template <typename prefix_tag_t>
    static constexpr auto parse_mangling(std::string_view value, std::string_view function)
    {
        value.remove_prefix(value.find(function) + std::size(function));
#if defined(__GNUC__) or defined(__clang__)
        value.remove_prefix(value.find(prefix_tag_t::value) + std::size(prefix_tag_t::value));
#if defined(__clang__)
        value.remove_suffix(value.length() - value.rfind(']'));
#elif defined(__GNUC__) // GCC
        value.remove_suffix(value.length() - value.find(';'));
#endif
#elif defined(_MSC_VER)
        value.remove_prefix(value.find('<') + 1);
        if (auto enum_token_pos = value.find("enum "); enum_token_pos == 0)
            value.remove_prefix(enum_token_pos + sizeof("enum ") - 1);
        value.remove_suffix(value.length() - value.rfind(">(void)"));
#endif
        return value;
    }
}

namespace gcl::cx::typeinfo
{ // constexpr typeinfo that does not relies on __cpp_rtti
    //
    // Known limitations :
    //  type_name : type aliases
    //      ex : std::string
    //          std::basic_string<char>
    //          std::__cxx11::basic_string<char>
    //          std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char>>
    //
    //  value_name : values representation
    //      ex : int(42)
    //          0x2a ont MsVC/CL
    //      use <charconv> std::to_chars into std::string_view for reliable basic numerical values

    template <typename T>
    static constexpr /*consteval*/ auto type_name(/*no parameters allowed*/) -> std::string_view
    {
#if defined(__GNUC__) or defined(__clang__)
        return details::parse_mangling<details::type_prefix_tag>(__PRETTY_FUNCTION__, __FUNCTION__);
#elif defined(_MSC_VER)
        return details::parse_mangling<details::type_prefix_tag>(__FUNCSIG__, __func__);
#else
        static_assert(false, "gcl::cx::typeinfo : unhandled plateform");
#endif
    }
    template <typename T>
    constexpr inline auto type_name_v = type_name<T>();
    template <auto value>
    static constexpr auto type_name(/*no parameters allowed*/) -> std::string_view
    {
        return type_name<decltype(value)>();
    }

    template <auto value>
    static constexpr auto value_name(/*no parameters allowed*/) -> std::string_view
    {
#if defined(__GNUC__) or defined(__clang__)
        return details::parse_mangling<details::value_prefix_tag>(__PRETTY_FUNCTION__, __FUNCTION__);
#elif defined(_MSC_VER)
        return details::parse_mangling<details::value_prefix_tag>(__FUNCSIG__, __func__);
#else
        static_assert(false, "gcl::cx::typeinfo : unhandled plateform");
#endif
    }
    template <auto value>
    constexpr inline auto value_name_v = value_name<value>();

    template <typename T>
    static constexpr auto hashcode()
    { // as template<> struct hash<std::string_view>; is not consteval
        constexpr auto type_name = gcl::cx::typeinfo::type_name<T>();
        return gcl::cx::crc_32::hash(type_name);
    }
    using hashcode_t = gcl::cx::hash_type;
    template <typename T>
    constexpr inline hashcode_t hashcode_v = hashcode<T>();
}

#include <array>
#include <gcl/mp/pack_traits.hpp>
namespace gcl::cx::typeinfo
{
    template <typename Type>
    constexpr auto to_hashcode_array()
    {
        using type_arguments_as_tuple = typename gcl::mp::pack_traits<Type>::template arguments_as<std::tuple>;
        using index_type = std::make_index_sequence<std::tuple_size_v<type_arguments_as_tuple>>;

        constexpr auto generate_mapping_impl =
            []<typename TupleType, std::size_t... Index>(TupleType, std::index_sequence<Index...>)
        {
            static_assert(sizeof...(Index) == std::tuple_size_v<TupleType>);

            using hash_type = decltype(gcl::cx::typeinfo::hashcode_v<std::tuple_element_t<0, TupleType>>);
            using mapping_type = std::array<hash_type, sizeof...(Index)>;

            return mapping_type{gcl::cx::typeinfo::hashcode_v<std::tuple_element_t<Index, TupleType>>...};
        };
        return generate_mapping_impl(type_arguments_as_tuple{}, index_type{});
    }
}

namespace gcl::cx::typeinfo::test
{
    // basic type
    static_assert(gcl::cx::typeinfo::type_name<int(42)>() == "int");
#if defined(__GNUC__) or defined(__clang__)
    static_assert(gcl::cx::typeinfo::value_name<int(42)>() == "42");
#else
    static_assert(gcl::cx::typeinfo::value_name<int(42)>() == "0x2a");
#endif

    // namespace, scoped
    enum global_ns_colors : int
    {
        red,
        blue,
        yellow,
        orange,
        green,
        purple
    };
    static_assert(gcl::cx::typeinfo::type_name<global_ns_colors::red>() == "gcl::cx::typeinfo::test::global_ns_colors");
    static_assert(gcl::cx::typeinfo::value_name<global_ns_colors::red>() == "gcl::cx::typeinfo::test::red");

    // to_hashcode_array
    template <typename... Ts>
    struct type_with_variadic_type_parameters {};
    using type = type_with_variadic_type_parameters<int, bool, float>;

    constexpr auto mapping = to_hashcode_array<type>();
    static_assert(mapping[0] == gcl::cx::typeinfo::hashcode_v<int>);
    static_assert(mapping[1] == gcl::cx::typeinfo::hashcode_v<bool>);
    static_assert(mapping[2] == gcl::cx::typeinfo::hashcode_v<float>);
}
