#ifndef GCL_TRAITS__
# define GCL_TRAITS__

namespace GCL
{
	namespace TypeTrait
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
				GCL::TypeTrait::TypeToUniqueId<A>::value == GCL::TypeTrait::TypeToUniqueId<A>::value
				&& GCL::TypeTrait::TypeToUniqueId<B>::value == GCL::TypeTrait::TypeToUniqueId<B>::value
				&& GCL::TypeTrait::TypeToUniqueId<A>::value != GCL::TypeTrait::TypeToUniqueId<B>::value
				;
			}
		};
	}
}

#endif // GCL_TRAITS__