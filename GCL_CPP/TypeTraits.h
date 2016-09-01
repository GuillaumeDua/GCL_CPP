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

		template<typename... T>
		std::vector<int> Make_TypeToUniqueIdVector() { return { TypeToUniqueId<T>::value... }; }

		struct Variant
		{
			template <typename T>
			explicit Variant(T & var)
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

		// Substitute to C++17 feature : constraint/require
		// Allow sfinae static type checking in C++TMP
#pragma region Waiting_for_Cpp17_constraint_require_reflexion
		namespace sfinae
		{
		    template<typename> struct Gen { typedef void type; };
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
