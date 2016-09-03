#ifndef GCL_TRAITS__
# define GCL_TRAITS__

# include "TypeTrait_SFINAE.hpp"

namespace GCL
{
	namespace TypeTrait
	{
		// Warning : Values change per program instance
		template <typename T>
		struct	TypeToUniqueId
		{
			using _Type = T;
			static const int value;
		};
		template <typename T>
		const int TypeToUniqueId<T>::value = reinterpret_cast<int>(&TypeToUniqueId<T>::value);

		template<typename... T>
		std::vector<int> Make_TypeToUniqueIdVector() { return { TypeToUniqueId<T>::value... }; }

		struct Any // Variant [?]
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
				{
					std::cerr << "[Warning] : Bad type requested : [" << _typeUniqueId << "] to [" << TypeToUniqueId<T>::value << ']' << std::endl;
					throw std::bad_cast;
				}
				return *reinterpret_cast<T*>(_ptr);
			}

		private:
			void *      _ptr;
			const int   _typeUniqueId;
		};

		template <class T, class Tuple>
		struct IndexOf;
		template <class T, class... _Types>
		struct IndexOf<T, std::tuple<T, _Types...> >
		{
			static const std::size_t value = 0;
		};
		template <class T, class U, class... _Types>
		struct IndexOf<T, std::tuple<U, _Types...> >
		{
			static const std::size_t value = 1 + IndexOf<T, std::tuple<_Types...> >::value;	// Todo : Check recusion
		};



		struct Test
		{
			struct A{}; struct B{};

			static bool Proceed(void)
			{
				std::vector<int> typeIdVector = Make_TypeToUniqueIdVector<int, char, int, char, std::string, A, B>();

				struct B {};
				using _Types = std::tuple<int, char, B>;

				return
					GCL::TypeTrait::TypeToUniqueId<A>::value == GCL::TypeTrait::TypeToUniqueId<A>::value
					&& GCL::TypeTrait::TypeToUniqueId<B>::value == GCL::TypeTrait::TypeToUniqueId<B>::value
					&& GCL::TypeTrait::TypeToUniqueId<A>::value != GCL::TypeTrait::TypeToUniqueId<B>::value
					&& typeIdVector.at(0) == typeIdVector.at(2)
					&& typeIdVector.at(1) == typeIdVector.at(3)
					&& IndexOf<B, _Types>::value == 2;
					;
			}
		};
	}
}

#endif // GCL_TRAITS__
