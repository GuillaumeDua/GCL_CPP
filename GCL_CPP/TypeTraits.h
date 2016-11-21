#ifndef GCL_TRAITS__
# define GCL_TRAITS__

# include "TypeTrait_SFINAE.hpp"
# include <vector>
# include <unordered_map>
# include <functional>
# include <iostream>
# include <string>

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

		template <typename ... Types>
		struct TypePack
		{
			using _Types = std::tuple<Types...>;

			template <typename T>
			static constexpr inline size_t indexOf(void)
			{
				return indexOf_impl<T, 0, Types...>();
			}

			template <size_t N>
			using TypeAt = typename std::tuple_element<N, _Types>::type;

			// Add any other function here

		private:
			template <typename T_Needle, size_t It, typename T_It, typename ... T_HayStack>
			static constexpr inline size_t indexOf_impl(void)
			{
				return (std::is_same<T_Needle, T_It>::value ? It : indexOf_impl<T_Needle, It + 1, T_HayStack...>());
			}
			template <typename T_Needle, size_t It>
			static constexpr inline size_t indexOf_impl(void)
			{
				throw std::out_of_range("TypeInfo::indexOf_impl : Not found");
			}
		};

		template <typename T_Interface>
		struct InterfaceIs
		{
			using _Interface = T_Interface;
			using index_type = size_t;
			using basic_container_type = typename std::unordered_map<index_type, std::function<_Interface*(void)>>;

			template <class T>
			struct CtorCaller
			{
				static const typename basic_container_type::mapped_type value;
			};

			template <typename ... Types>
			struct OfTypes
			{
				// TODO : static_assert(std::is_base_of<_Interface, T>::value && std::is_constructible<T>::value)

				using T_TypePack = TypePack<Types...>;

				template <typename T>
				struct _Elem : basic_container_type::value_type
				{
					_Elem()
						: basic_container_type::value_type{ TypePack<Types...>::template indexOf<T>(), std::ref(CtorCaller<T>::value) }
					{}
				};

				struct Indexer
					: public std::unordered_map<index_type, std::function<T_Interface*(void)>> // [TODO] : replace std::function by a struct with many tools in [?]
				{
					explicit Indexer()
						: basic_container_type{ _Elem<Types>()... }
					{}

					template <typename T>
					static inline constexpr index_type indexOf(void)
					{
						return TypePack<Types...>::template indexOf<T>();
					}
					template <typename T>
					inline typename basic_container_type::mapped_type at(void) const
					{
						return basic_container_type::at(indexOf<T>());
					}
				} /*static index*/; // [TODO]::[FixMe] : How to duplicate pack expansion [?]
				static const basic_container_type index;
			};
		};

		static bool Test_interfaceIs(void)
		{
			struct Interface { virtual const std::string name() = 0; };
			struct Toto : Interface { const std::string name() { return "Toto"; } };
			struct Titi : Interface { const std::string name() { return "Titi"; } };
			struct Tata : Interface { const std::string name() { return "Tata"; } };
			struct Tutu : Interface { const std::string name() { return "Tutu"; } };
			struct NotInPack {};

			try
			{
				TypePack<Toto, Titi, Tata, Tutu>::indexOf<Tata>();
				TypePack<Toto, Titi, Tata, Tutu>::indexOf<NotInPack>();	// throw std::out_of_range
				return false;
			}
			catch (const std::out_of_range &)
			{}

			try
			{
				InterfaceIs<Interface>::OfTypes<Toto, Titi, Tata, Tutu>::Indexer index;

				for (auto & elem : index)
					std::cout << "\t- [" << elem.first << "] => [" << elem.second()->name() << ']' << std::endl;
				if (index.size() != InterfaceIs<Interface>::OfTypes<Toto, Titi, Tata, Tutu>::index.size())
					throw std::exception("Sizes mismatch");
				return true;
			}
			catch (const std::exception & ex)
			{
				std::cerr << "Exception catched : " << ex.what() << std::endl;
			}
			catch (...)
			{
				std::cerr << "Unknown stuff catched" << std::endl;
			}
			return false;
		}

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
					//&& GCL::TypeTrait::IndexOf<B, _Types>::value == 2
					&& Test_interfaceIs()
					;
			}
		};
	}
}

#endif // GCL_TRAITS__
