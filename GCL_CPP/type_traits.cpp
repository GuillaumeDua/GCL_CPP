#include <gcl_cpp/type_traits.hpp>

namespace gcl
{
	namespace type_trait
	{
		template <typename T_Interface>
		template <class T>
		const typename interface_is<T_Interface>::TypeHelper::CB_DefaultCtorCallerType
			interface_is<T_Interface>::TypeHelper::DefaultCtorCaller<T>::value = { []() -> T_Interface* { return new T(); } };

		template <typename T_Interface>
		template <typename ... Types>
		const std::unordered_map<size_t, typename interface_is<T_Interface>::TypeHelper> interface_is<T_Interface>::of_types<Types...>::index = { _Elem<Types>()... };
	}
}
