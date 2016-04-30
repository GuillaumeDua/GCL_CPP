#ifndef GCL_EXPERIMENTAL__
# define GCL_EXPERIMENTAL__

# include "TypeTraits.h"
# include <functional>

/*
	/!\ BEWARE /!\
	This is experimental, uncomplete features, that may become GCL components in the futur. (And thus move to other files).
*/

using namespace GCL::TypeTrait;

namespace GCL
{
	namespace Experimental
	{
		namespace NestedTypeWrapper
		{
			template <typename T_VarOriginType>
			struct NonStaticMember
			{
				static void Call(void * ptr, std::function<void(T_VarOriginType&)> fun)
				{
					fun(reinterpret_cast<T_VarOriginType&>(*ptr));
				}
			};
		}
		namespace TypeTrait
		{
			struct NullType{};

			template <int value, typename T = NullType>
			struct	UniqueIdToType
			{
				using _Type = T;
				static const int value;
			};

			template <template <typename> class T_VisitorType>
			void	VisitUniqueIdVar(int varUniqueId, void * var)
			{
				// using _VarType = GetTypeByUniqueId
				T_VisitorType<_VarType>::Visit(var);
			};

			struct _TypeIdListConfiguration
			{
				static const int _NoValue = -1;
				static const int _MaxIteration = 100;
			};

			// TODO : Generic design : "MapStaticToNonStaticValues<StaticValue, NonStaticValue>
			template <int _I, typename T_Configuration = _TypeIdListConfiguration>
			struct TypeIdList
			{
				enum { value = _I };

				static int _uniqueId;

				using _Configuration = T_Configuration;

				using _Type = TypeIdList < _I > ;
				using _Next = TypeIdList < _I + 1 > ;
				using _Prev = TypeIdList < _I - 1 > ;

				template <template <typename> class T_TypeIdList_Visitor, int _MaxIt = _Configuration::_MaxIteration>
				struct Visit
				{
					template <bool _Continue = true>
					static void	Do(void)
					{
						// static_assert((_MaxIt <= _Configuration::_MaxIteration));

						if (_uniqueId != _Configuration::_NoValue)
						{
							T_TypeIdList_Visitor<_Type>::Visit();
							_Next::Visit<T_TypeIdList_Visitor, _MaxIt>::Do < (_I < _MaxIt - 1)>();
						}
					}
					template <>
					static void	Do<false>(void)
					{}
				};
			};
			template <int _I, typename T_Configuration = _TypeIdListConfiguration>
			int TypeIdList<_I, T_Configuration>::_uniqueId = TypeIdList<_I, T_Configuration>::_Configuration::_NoValue;

			template <typename T, typename T_TypeId_List = TypeIdList<0> >
			struct RegisterType
			{
				template <bool _Continue = true>
				static void	Do(void)
				{
					if (typename T_TypeId_List::_uniqueId != T_TypeId_List::_Configuration::_NoValue)
						RegisterType<T, T_TypeId_List::_Next>::Do < (T_TypeId_List::value < T_TypeId_List::_Configuration::_MaxIteration - 1)>();
					else
					{
						T_TypeId_List::_uniqueId = TypeToUniqueId<T>::value;
					}
				}
				template <>
				static void	Do<false>(void)
				{}
			};

			template <typename T_TypeList>
			struct TestTypeListVisitor_Dump
			{
				static void	Visit(void)
				{
					std::cout << "value=[" << T_TypeList::value << "], UniqueId=[" << T_TypeList::_uniqueId << ']' << std::endl;
				}
			};

			struct Test
			{
				struct A{}; struct B{};

				static bool Proceed(void)
				{
					RegisterType<char>::Do();			// generate : TypeIdList<0>::_uniqueId = TypeToUniqueId<char>::value;
					RegisterType<std::string>::Do();	// generate : TypeIdList<1>::_uniqueId = TypeToUniqueId<std::string>::value;
					RegisterType<A>::Do();				// generate : TypeIdList<2>::_uniqueId = TypeToUniqueId<A>::value;

					TypeIdList<0>::Visit<TestTypeListVisitor_Dump>::Do();

					return
						GCL::TypeTrait::TypeToUniqueId<A>::value == GCL::TypeTrait::TypeToUniqueId<A>::value
						&& GCL::TypeTrait::TypeToUniqueId<B>::value == GCL::TypeTrait::TypeToUniqueId<B>::value
						&& GCL::TypeTrait::TypeToUniqueId<A>::value != GCL::TypeTrait::TypeToUniqueId<B>::value
						;
				}
			};
		}
		namespace Inheritance
		{
			template <class T, class ...Classes>
			struct Super
			{
				struct Type : T, Super<Classes...>::Type
				{};
			};
			template <class T>
			struct Super<T>
			{
				using Type = T;
			};


			struct Test
			{
				struct A { A() { std::cout << "A" << std::endl; } };
				struct B { B() { std::cout << "B" << std::endl; } };
				struct C { C() { std::cout << "C" << std::endl; } };

				static bool Proceed()
				{

					Super<A, B, C>::Type	super;

					return std::is_convertible<Super<A, B, C>::Type, A>::value
						&& std::is_convertible<Super<A, B, C>::Type, B>::value
						&& std::is_convertible<Super<A, B, C>::Type, C>::value
						;
				}
			};
		}

		// Epic : TypeToId + IdToType
		namespace Puzzle
		{
			struct NullType {};

			template <typename T>
			struct TypeToID
			{
				static const size_t value = 42;
			};
			template <size_t ID>
			struct IDToType
			{
				using _Type = NullType;
			};

			struct Test
			{
				static bool Proceed()
				{
					struct Toto {};

					const size_t id = TypeToID<Toto>::value;

					return (
						std::is_same<IDToType<id>::_Type, Toto>::value
						);
				}
			};
		}
	}
}

#endif // GCL_EXPERIMENTAL__