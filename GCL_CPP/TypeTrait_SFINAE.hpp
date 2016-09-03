#ifndef GCL_TRAITS_SFINAE__
# define GCL_TRAITS_SFINAE__

namespace GCL
{
	namespace TypeTrait
	{
		// Substitute to C++17 feature : constraint/require
		// Allow sfinae static type checking in C++TMP
#pragma region Waiting_for_Cpp17_constraint_require_reflexion
		namespace sfinae
		{
			template<typename> struct Gen { typedef void type; };
		}
	}
}
#define gen_has_nested_type(name)                                   \
namespace GCL                                                       \
{                                                                   \
    namespace TypeTrait                                             \
    {                                                               \
        namespace sfinae                                            \
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
        }                                                           \
    }                                                               \
}
#define gen_has_member_function(name)                               \
namespace GCL                                                       \
{                                                                   \
    namespace TypeTrait                                             \
    {                                                               \
        namespace sfinae                                            \
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
        }                                                           \
    }                                                               \
}
#pragma endregion Waiting_for_Cpp17_constraint_require_reflexion

#endif // GCL_TRAITS_SFINAE__
