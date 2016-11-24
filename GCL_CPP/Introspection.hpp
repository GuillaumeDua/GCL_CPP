#ifndef GCL_INTROSPECTION
# define GCL_INTROSPECTION

# include "Preprocessor.h"

// [Todo] : Introspection or Reflexion [?]
// [Todo] : Use ParamTester insteand of std::void_t to make synthax clearer [?]

#define GCL_Introspection__GenHasNested(nested)									\
namespace GCL { namespace Introspection {										\
template< class, class = std::void_t<> >										\
struct has_##nested##_nested : std::false_type { };								\
template< class T >																\
struct has_##nested##_nested<T, std::void_t<typename T::##nested>> : std::true_type { };	\
}}

#define GCL_Introspection__GenHasMemberFunction(name)							\
namespace GCL { namespace Introspection {										\
template< class, class = std::void_t<> >										\
struct has_##name##_memberFunction : std::false_type { };						\
template< class T >																\
struct has_##name##_memberFunction<T, std::void_t<decltype(&T:: name)>> : std::true_type { };	\
}}

DEBUG_INSTRUCTION
(	// For test purpose only
	// Nested
	GCL_Introspection__GenHasNested(Flag1);
	GCL_Introspection__GenHasNested(Flag2);
	GCL_Introspection__GenHasNested(Flag3);
	// MemberFunction
	GCL_Introspection__GenHasMemberFunction(doStuff);
	);

namespace GCL
{
	namespace Introspection
	{
		struct Test
		{
			static bool Proceed(void)
			{
				RELEASE_INSTRUCTION(return true;);
				DEBUG_INSTRUCTION
				(
					struct Toto { enum		Flag1 {}; void doStuff() {} };
					struct Titi { struct	Flag2 {}; };
					struct Tata { enum		Flag3 {}; };

					return
					(
						//=> Nested :
						// Flag1
							GCL::Introspection::has_Flag1_nested<Toto>::value
						&& !GCL::Introspection::has_Flag2_nested<Toto>::value
						&& !GCL::Introspection::has_Flag3_nested<Toto>::value
						// Flag2
						&& !GCL::Introspection::has_Flag1_nested<Titi>::value
						&&	GCL::Introspection::has_Flag2_nested<Titi>::value
						&& !GCL::Introspection::has_Flag3_nested<Titi>::value
						// Flag3
						&& !GCL::Introspection::has_Flag1_nested<Tata>::value
						&& !GCL::Introspection::has_Flag2_nested<Tata>::value
						&&	GCL::Introspection::has_Flag3_nested<Tata>::value
						//=> MemberFunction :
						// doStuff
						&&	GCL::Introspection::has_doStuff_memberFunction<Toto>::value
						&& !GCL::Introspection::has_doStuff_memberFunction<Titi>::value
						&& !GCL::Introspection::has_doStuff_memberFunction<Tata>::value
					);	// NB : This boolean is constexpr
					)
			}
		};
	}
}

//
//	Previous version :
//

namespace GCL
{
	namespace OLD {
		namespace TypeTrait
		{
			// Allow sfinae static type introspection in C++TMP
#pragma region Waiting_for_Cpp17_constraint_require_reflexion
			namespace sfinae
			{
				template<typename> struct Gen { typedef void type; };
			}
		}
	}
}
#define gen_has_nested_type(name)                               \
namespace GCL {                                                 \
	namespace OLD {												\
    namespace TypeTrait {                                       \
    namespace sfinae											\
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
    }}}                                                         \
}
#define gen_has_member_function(name)                           \
namespace GCL {													\
	namespace OLD {												\
    namespace TypeTrait {										\
    namespace sfinae											\
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
    }}}															\
}
#pragma endregion Waiting_for_Cpp17_constraint_require_reflexion

#endif // GCL_INTROSPECTION
