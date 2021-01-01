#ifndef GCL_INTROSPECTION
# define GCL_INTROSPECTION

# include <gcl_cpp/preprocessor.hpp>

#define GCL_Introspection_Generate__has_nested(nested)							\
namespace gcl::introspection::generated::has_nested_type{						\
	template< class, class = std::void_t<> >									\
	struct nested : std::false_type { };										\
	template< class T >															\
	struct nested##<T, std::void_t<typename T::##nested>> : std::true_type { };	\
}

#define GCL_Introspection_Generate__has_member_function(name)					\
namespace gcl::introspection::generated::has_member_function{					\
	template< class, class = std::void_t<> >									\
	struct name## : std::false_type { };										\
	template< class T >															\
	struct name##<T, std::void_t<decltype(&T:: name)>> : std::true_type { };	\
}

namespace gcl::deprecated::introspection
{
	// Allow sfinae static type introspection in C++TMP
	template<typename> struct Gen { typedef void type; };
}

#define GCL_DEPRECATED_INTROSPECTION__gen_has_nested_type(name) \
namespace gcl::deprecated::introspection::generated			\
{                                                           \
    template<typename T, typename SFINAE = void>            \
    struct HasNestedType_##name : std::false_type {};       \
                                                            \
    template<typename T>                                    \
    struct HasNestedType_##name                             \
    <                                                       \
            T                                               \
        ,   typename Gen                                    \
            <                                               \
                typename T::NestedType                      \
            >::type                                         \
    > : std::true_type {};                                  \
                                                            \
    template <class T>                                      \
    constexpr inline bool has_nested_type_##name(void)      \
    {                                                       \
        return std::is_same<typename HasNestedType_##name<T>::type, std::true_type>::value; \
    }                                                       \
}

#define GCL_DEPRECATED_INTROSPECTION__gen_has_member_function(name) \
namespace gcl::deprecated::introspection::generated			\
{                                                           \
    template<typename T, typename SFINAE = void>            \
    struct HasMemberFunction##name : std::false_type {};    \
                                                            \
    template<typename T>                                    \
    struct HasMemberFunction##name                          \
    <                                                       \
            T                                               \
        ,   typename Gen                                    \
            <                                               \
                decltype(&T:: name)                         \
            >::type                                         \
    > : std::true_type {};                                  \
                                                            \
    template <class T>                                      \
    constexpr inline bool has_member_function_##name(void)  \
    {                                                       \
        return std::is_same<typename HasMemberFunction##name<T>::type, std::true_type>::value; \
    }                                                       \
}

#endif // GCL_INTROSPECTION
