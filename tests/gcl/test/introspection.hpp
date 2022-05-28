#pragma once

#include <gcl_cpp/introspection.hpp>

// Nested
GCL_Introspection_Generate__has_nested(Flag1);
GCL_Introspection_Generate__has_nested(Flag2);
GCL_Introspection_Generate__has_nested(Flag3);
// MemberFunction
GCL_Introspection_Generate__has_member_function(doStuff);

namespace gcl::test
{
	struct introspection
	{
		struct Toto { enum		Flag1 {}; void doStuff() {} };
		struct Titi { struct	Flag2 {}; };
		struct Tata { enum		Flag3 {}; };

		static constexpr bool proceed(void)
		{
			return
				(
					//=> Nested :
					// Flag1
					gcl::introspection::generated::has_nested_type::Flag1<Toto>::value
					&& !gcl::introspection::generated::has_nested_type::Flag2<Toto>::value
					&& !gcl::introspection::generated::has_nested_type::Flag3<Toto>::value
					// Flag2
					&& !gcl::introspection::generated::has_nested_type::Flag1<Titi>::value
					&&	gcl::introspection::generated::has_nested_type::Flag2<Titi>::value
					&& !gcl::introspection::generated::has_nested_type::Flag3<Titi>::value
					// Flag3
					&& !gcl::introspection::generated::has_nested_type::Flag1<Tata>::value
					&& !gcl::introspection::generated::has_nested_type::Flag2<Tata>::value
					&&	gcl::introspection::generated::has_nested_type::Flag3<Tata>::value
					//=> MemberFunction :
					// doStuff
					&&	gcl::introspection::generated::has_member_function::doStuff<Toto>::value
					&& !gcl::introspection::generated::has_member_function::doStuff<Titi>::value
					&& !gcl::introspection::generated::has_member_function::doStuff<Tata>::value
					);	// NB : This boolean is constexpr
		}
	};
}