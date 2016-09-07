#include "TypeTraits.h"

namespace GCL
{
	namespace TypeTrait
	{
		template <typename T_Interface>
		template <class T>
		const typename InterfaceIs<T_Interface>::basic_container_type::mapped_type
			InterfaceIs<T_Interface>::CtorCaller<T>::value = { []() -> T_Interface* { return new T(); } };

		template <typename T_Interface>
		template <typename ... Types>
		const std::unordered_map<size_t, std::function<T_Interface*(void)>> InterfaceIs<T_Interface>::OfTypes<Types...>::index = { _Elem<Types>()... };
	}
}
