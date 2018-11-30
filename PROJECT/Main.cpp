// #define _GCL_DEBUG

# include <gcl_cpp/type_index.hpp>
namespace gcl::deprecated::type_index
{	// type_index.cpp content
	template <typename T_Interface>
	template <class T>
	const typename interface_is<T_Interface>::type_helper::default_constructor_t
		interface_is<T_Interface>::type_helper::get_default_constructor_t<T>::value = { []() -> T_Interface* { return new T(); } };

	template <typename T_Interface>
	template <typename ... Types>
	const std::unordered_map<size_t, typename interface_is<T_Interface>::type_helper> interface_is<T_Interface>::of_types<Types...>::index = { element<Types>()... };
}

# include <gcl_cpp/maths.h>
# include <gcl_cpp/mp.hpp>
# include <gcl_cpp/serialisation.hpp>

#include <gcl_cpp/test/gcl.hpp>
#include <gcl_cpp/signals.hpp>

auto	main(int, char const * []) -> int
{
	try
	{
		gcl::signals::initialize();
		gcl::test::proceed();
	}
	catch (const std::exception & ex)
	{
		std::cerr << "Error : " << ex.what() << std::endl;
	}
	catch (...)
	{
		std::cerr << "Error : unknown" << std::endl;
	}

	// todo : is_constexpr trait

#ifdef _WIN32
	system("pause");
#endif
	return 0;
}