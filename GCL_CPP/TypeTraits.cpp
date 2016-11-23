#include "TypeTraits.h"

namespace GCL
{
	namespace TypeTrait
	{
		template <typename T_Interface>
		template <class T>
		const typename InterfaceIs<T_Interface>::TypeHelper::CB_DefaultCtorCallerType
			InterfaceIs<T_Interface>::TypeHelper::DefaultCtorCaller<T>::value = { []() -> T_Interface* { return new T(); } };

		template <typename T_Interface>
		template <typename ... Types>
		const std::unordered_map<size_t, typename InterfaceIs<T_Interface>::TypeHelper> InterfaceIs<T_Interface>::OfTypes<Types...>::index = { _Elem<Types>()... };
	}
}
