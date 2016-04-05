#ifndef GCL_TRAITS__
# define GCL_TRAITS__

namespace GCL
{
	namespace Traits
	{
		template <typename T>
		struct	TypeToUniqueId
		{
			using _Type = T;
			static const int value;

		};
		template <typename T>
		const int TypeToUniqueId<T>::value = reinterpret_cast<int>(&TypeToUniqueId<T>::value);

		template <int TypeId>
		struct IntToType
		{
			// [Todo]
		};

		struct Test
		{
			struct A{}; struct B{};

			static bool Proceed(void) _noexcept
			{
				return
				GCL::Traits::TypeToUniqueId<A>::value == GCL::Traits::TypeToUniqueId<A>::value
				&& GCL::Traits::TypeToUniqueId<B>::value == GCL::Traits::TypeToUniqueId<B>::value
				&& GCL::Traits::TypeToUniqueId<A>::value != GCL::Traits::TypeToUniqueId<B>::value
				;
			}
		};
	}
}

#endif // GCL_TRAITS__