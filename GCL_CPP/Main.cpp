// #define _GCL_DEBUG

# include <gcl_cpp/type_index.hpp>
# include <gcl_cpp/task.hpp>
# include <gcl_cpp/maths.h>
# include <gcl_cpp/ownership.hpp>
# include <gcl_cpp/Experimental.h>
# include <gcl_cpp/mp.hpp>
# include <gcl_cpp/serialisation.hpp>
# include <gcl_cpp/pattern.hpp>
# include <gcl_cpp/TestUtils.h>

//
// Link symbols :
//
namespace gcl
{
	namespace experimental
	{
		namespace test_utils
		{
			std::mutex									Inline::RT_scope_controler::_container_mutex;
			std::mutex									Inline::RT_scope_controler::_obs_mutex;
			volatile bool								Inline::RT_scope_controler::_isActive;
			Inline::RT_scope_controler::T_Container		Inline::RT_scope_controler::_container;
			Inline::RT_scope_controler::T_ObserversList	Inline::RT_scope_controler::_observers;
		}
	}
	namespace type_index
	{
		template <typename T_Interface>
		template <class T>
		const typename interface_is<T_Interface>::type_helper::default_constructor_t
			interface_is<T_Interface>::type_helper::get_default_constructor_t<T>::value = { []() -> T_Interface* { return new T(); } };

		template <typename T_Interface>
		template <typename ... Types>
		const std::unordered_map<size_t, typename interface_is<T_Interface>::type_helper> interface_is<T_Interface>::of_types<Types...>::index = { element<Types>()... };
	}
}

# include <gcl_cpp/test/gcl.hpp>


auto	main(int, char const * []) -> int
{
	//Tool::Test::TestMultipleComponents
	//<
	//	gcl::serialization::Test
	//	, gcl::experimental::type_index::Test
	//	, gcl::mp::Test
	//	, gcl::type_index::Test
	//	, gcl::Introspection::Test
	//	
	//	//, gcl::task::Test
	//	//, gcl::Events::Test
	//	//, gcl::Container::Test
	//	//, gcl::ownership::Test

	//	, gcl::experimental::type_index::Test
	//	// gcl::experimental::Puzzle::Test
	//	// , gcl::Color::Test // FIXME
	//	, gcl::experimental::pattern::Test
	//	, gcl::experimental::test_utils::Inline::Test
	//>();
	gcl::test::proceed();

	// todo : is_constexpr trait

	system("pause");
	return 0;
}