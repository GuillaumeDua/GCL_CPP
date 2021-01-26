#pragma once

#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <gcl/cx/crc32_hash.hpp>

namespace gcl::mp::typeinfo
{   // constexpr typeinfo that does not relies on __cpp_rtti
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
    static constexpr /*consteval*/ std::string_view type_name(/*no parameters allowed*/)
    {
#if defined(__GNUC__)
        std::string_view str_view = __PRETTY_FUNCTION__;
        str_view.remove_prefix(str_view.find(__FUNCTION__) + sizeof(__FUNCTION__));
        const char prefix[] = "T = ";
        str_view.remove_prefix(str_view.find(prefix) + sizeof(prefix) - 1);
        str_view.remove_suffix(str_view.length() - str_view.find_first_of(";]"));
#elif defined(_MSC_VER)
        std::string_view str_view = __FUNCSIG__;
        str_view.remove_prefix(str_view.find(__func__) + sizeof(__func__));
        if (auto enum_token_pos = str_view.find("enum "); enum_token_pos == 0)
            str_view.remove_prefix(enum_token_pos + sizeof("enum ") - 1);
        str_view.remove_suffix(str_view.length() - str_view.rfind(">(void)"));
#else
        static_assert(false, "gcl::mp::typeinfo : unhandled plateform");
#endif
        return str_view;
    }
    template <typename T>
    static constexpr inline auto type_name_v = type_name<T>();
    template <auto value>
    static constexpr std::string_view type_name(/*no parameters allowed*/)
    {
        return type_name<decltype(value)>();
    }

    template <auto value>
    static constexpr std::string_view value_name(/*no parameters allowed*/)
    {
#if defined(__GNUC__)
        std::string_view str_view = __PRETTY_FUNCTION__;
        str_view.remove_prefix(str_view.find(__FUNCTION__) + sizeof(__FUNCTION__));
        const char prefix[] = "value = ";
        str_view.remove_prefix(str_view.find(prefix) + sizeof(prefix) - 1);
        str_view.remove_suffix(str_view.length() - str_view.find_first_of(";]"));
#elif defined(_MSC_VER)
        std::string_view str_view = __FUNCSIG__;
        str_view.remove_prefix(str_view.find(__func__) + sizeof(__func__));
        if (auto enum_token_pos = str_view.find("enum "); enum_token_pos == 0)
            str_view.remove_prefix(enum_token_pos + sizeof("enum ") - 1);
        str_view.remove_suffix(str_view.length() - str_view.rfind(">(void)"));
#else
        static_assert(false, "gcl::mp::typeinfo : unhandled plateform");
#endif
        return str_view;
    }
    template <auto value>
    static constexpr inline auto value_name_v = value_name<value>();

    template <typename T>
    static constexpr auto hashcode()
    {   // as template<> struct hash<std::string_view>; is not consteval
        constexpr auto type_name = gcl::mp::typeinfo::type_name<T>();
        return gcl::cx::crc_32::hash(type_name);
    }
    template <typename T>
    static constexpr inline auto hashcode_v = hashcode<T>();
}

#include <gcl/mp/pack_traits.hpp>
#include <array>
namespace gcl::mp::typeinfo
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

            using hash_type = decltype(gcl::mp::typeinfo::hashcode_v<std::tuple_element_t<0, TupleType>>);
            using mapping_type = std::array<hash_type, sizeof...(Index)>;

            return mapping_type{gcl::mp::typeinfo::hashcode_v<std::tuple_element_t<Index, TupleType>>...};
        };
        return generate_mapping_impl(type_arguments_as_tuple{}, index_type{});
    }
}

namespace gcl::mp::typeinfo::test
{
     // basic type
     static_assert(gcl::mp::typeinfo::type_name<int(42)>() == "int");
#if defined(_MSC_VER)
     static_assert(gcl::mp::typeinfo::value_name<int(42)>() == "0x2a");
#else
     static_assert(gcl::mp::typeinfo::value_name<int(42)>() == "42");
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
     static_assert(gcl::mp::typeinfo::type_name<global_ns_colors::red>() == "gcl::mp::typeinfo::test::global_ns_colors");
     static_assert(gcl::mp::typeinfo::value_name<global_ns_colors::red>() == "gcl::mp::typeinfo::test::red");

     // to_hashcode_array
     template <typename ... Ts>
     struct type_with_variadic_type_parameters{};
     using type = type_with_variadic_type_parameters<int, bool, float>;

     constexpr auto mapping = to_hashcode_array<type>();
     static_assert(mapping[0] == gcl::mp::typeinfo::hashcode_v<int>);
     static_assert(mapping[1] == gcl::mp::typeinfo::hashcode_v<bool>);
     static_assert(mapping[2] == gcl::mp::typeinfo::hashcode_v<float>);
}