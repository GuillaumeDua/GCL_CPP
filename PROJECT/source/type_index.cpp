#include <gcl_cpp/type_index.hpp>

namespace gcl::type_index
{
	template <typename T_Interface>
	template <class T>
	const typename interface_is<T_Interface>::type_helper::default_constructor_t
		interface_is<T_Interface>::type_helper::get_default_constructor_t<T>::value = { []() -> T_Interface* { return new T(); } };

	template <typename T_Interface>
	template <typename ... Types>
	const std::unordered_map<size_t, typename interface_is<T_Interface>::type_helper> interface_is<T_Interface>::of_types<Types...>::index = { element<Types>()... };
}
