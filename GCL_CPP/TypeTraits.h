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

		struct Any
		{
			template <typename T>
			explicit Any(T & var)
				: _ptr(&var)
				, _typeUniqueId(TypeToUniqueId<T>::value)
			{}

			template <typename T>
			inline T & GetAs(void)
			{
				if (TypeToUniqueId<T>::value != _typeUniqueId)
					std::cerr << "[Warning] : Bad type requested : [" << _typeUniqueId << "] to [" << TypeToUniqueId<T>::value << ']' << std::endl;
				return *reinterpret_cast<T*>(_ptr);
			}

		private:
			void *      _ptr;
			const int   _typeUniqueId;
		};

		struct Test
		{
			struct A{}; struct B{};

			static bool Proceed(void)
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