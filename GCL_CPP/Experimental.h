#ifndef GCL_EXPERIMENTAL__
# define GCL_EXPERIMENTAL__

# include "TypeTraits.h"
# include "TemplateMetaProgramming.h"
# include <functional>
# include <cassert>
# include <tuple>
# include <initializer_list>
# include <type_traits>

/*
	/!\ BEWARE /!\
	This is experimental, uncomplete features, that may become GCL components in the futur. (And thus move to other files).
*/

using namespace GCL::TypeTrait;
using namespace GCL::TMP;

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

			struct GenerateNextTypeID
			{
				static const size_t	Get()
				{
					return Type2ID_inc;
				}
				static const size_t GetInc(void)
				{
					return ++Type2ID_inc;
				}
				static size_t Type2ID_inc;
			};
			size_t GenerateNextTypeID::Type2ID_inc = 0;

			template <size_t ID>
			struct ID2Type
			{
				using _Type = NullType;
			};

			template <size_t & id, typename T>
			struct TypeAndTypeId
			{
				using _Type = T;
				static size_t & _ID;
			};
			template <size_t & id, typename T>
			size_t & TypeAndTypeId<id, T>::_ID = id;

			template <typename T>
			struct Type2ID
			{
				static /*const */size_t value;
				static size_t id;
			};
			template <typename T>
			size_t Type2ID<T>::value = GenerateNextTypeID::GetInc();
			template <typename T>
			size_t Type2ID<T>::id = TypeAndTypeId<value, T>::_ID;

			template <size_t id>
			auto	IsType(void)
			{
				return TypeAndTypeId<id, T>::id == id;
			}

			struct Test
			{
				static bool Proceed()
				{
					std::cout
						<< Type2ID<std::string>::id		<< std::endl
						<< Type2ID<std::string>::value	<< std::endl
						<< Type2ID<int>::id				<< std::endl
						<< Type2ID<int>::value			<< std::endl
						<< Type2ID<char>::id			<< std::endl
						<< Type2ID<char>::value			<< std::endl
						;
					

					return true;

					/*struct Toto {};*/
					//const size_t id = Type2ID<Toto>::value;
					//return (
					//	std::is_same<ID2Type<id>::_Type, Toto>::value
					//	);
				}
			};
		}
		namespace Puzzle2
		{
			struct ListableTypeCounter { static size_t _IDCounter; };
			size_t ListableTypeCounter::_IDCounter = 0;

			struct TypeByIndexFactory
			{
				using _GeneratorFunc = std::function<TypeByIndexFactory*(void)>;
				static std::vector < _GeneratorFunc >	_listableTypegenerator;
			};
			std::vector < TypeByIndexFactory::_GeneratorFunc >	TypeByIndexFactory::_listableTypegenerator;

			template <typename T>
			struct ListableType : TypeByIndexFactory
			{
				ListableType()
				{
					_ID;
					decltype(_listableTypegenerator_insert._enforceCodegeneration) enforceCodeGen = _listableTypegenerator_insert._enforceCodegeneration;
				}

				static size_t _ID;
				struct	_ListableTypegenerator_insert
				{
					_ListableTypegenerator_insert()
					{
						std::cout << "_ListableTypegenerator_insert" << std::endl;
						assert(_listableTypegenerator.size() == (_ID - 1));
						_listableTypegenerator.push_back([]() -> TypeByIndexFactory * {
							return new T;
						});
					}
				public:
					int _enforceCodegeneration = 0;
				}		static _listableTypegenerator_insert;
				
			};
			template <typename T>
			size_t ListableType<T>::_ID = (++ListableTypeCounter::_IDCounter);
			template <typename T>
			typename ListableType<T>::_ListableTypegenerator_insert ListableType<T>::_listableTypegenerator_insert;
			
			// std::tuple / std::tie / std::tuple_cat / std::make_tuple
			struct Test
			{
				static bool Proceed(void)
				{
					struct A : ListableType<A>
					{};

					std::cout << "Generated type   : " << ListableTypeCounter::_IDCounter << std::endl;
					std::cout << "Type to generate : " << TypeByIndexFactory::_listableTypegenerator.size() << std::endl;

					return true;
				}
			};
		}
		namespace Puzzle3
		{
			template <typename T_Functor>
			typename T_Functor::result_type	my_func(T_Functor & func)
			{
				return func();
			}
			
			
			template <typename T>
			struct TypenamePrint
			{
				static void Call(void)
				{
					std::cout << "Type : " << typeid(T).name() << std::endl;
				}
			};

			struct Super {};
			template <typename T>
			struct CtorCaller
			{
				using return_type = Super*;
				static return_type	Call(void)
				{
					return reinterpret_cast<return_type>(new T);
				}
			};

			struct Test
			{
				static bool Proceed()
				{
					struct A : Super {}; struct B : Super {}; struct C : Super {};
					using _Types = typename TypeContainer
						<
						A
						, B
						, C
						>;

					Foreach<A, B, C>::Call<TypenamePrint>();
					Foreach<_Types>::CallAt<TypenamePrint>(0);
					Super * super = Foreach<_Types>::CallAt_withReturn<CtorCaller>(0);
					delete super;

					std::function<int()> functor = []() { return 42; };
					std::cout << my_func(functor) << std::endl;

					return
						_Types::IndexOf<B>() == _Types::IndexOf<_Types::TypeAt<1>>()
						;
				}
			};
		}
	}
}

#endif // GCL_EXPERIMENTAL__