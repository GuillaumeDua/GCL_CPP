// #define _GCL_DEBUG

# include "TypeTraits.h"
# include "Task.h"
# include "Maths.h"
# include "EventHandler.h"
# include "SelfUpdate.h"
# include "Ownership.h"
# include "Experimental.h"
# include "TemplateMetaProgramming.h"
# include "Color.h"
# include "Serialization.h"
# include "Pattern.hpp"
# include "TestUtils.h"

//
// Link symbols :
//
namespace GCL
{
	namespace Experimental
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

# include <chrono>
# include <thread>
# include <iomanip>
# include <typeinfo>
# include <sstream>

namespace Tool
{
	// todo : replace by new test component
	struct Test
	{
		static uint32_t callCount;
		template <typename T_ComponentTest>
		struct TestOneComponent
		{
			static bool	Call(void)
			{
				++callCount;
				const std::string & symbol_name = typeid(T_ComponentTest).name();
				std::ostringstream componentTagName;
				componentTagName
					<< std::left
					<< std::setw(45)
					<< symbol_name
					;

				try
				{
					std::cout << "[Test] : [" << componentTagName.str() << "] : Processing ... " << std::endl;

					using T_ClockSystem = std::chrono::high_resolution_clock;

					T_ClockSystem::time_point tp_start = T_ClockSystem::now();
					bool ret = T_ComponentTest::Proceed();
					const long long elasped_usec = std::chrono::duration_cast<std::chrono::microseconds>(T_ClockSystem::now() - tp_start).count();

					std::cout << "[Test] : [" << componentTagName.str() << "] : .............. : [" << (ret ? "PASSED" : "FAILED") << "] in " << elasped_usec << "ms" << std::endl << std::endl;

					return ret;
				}
				catch (const std::exception & ex)
				{
					std::cerr
						<< "[Error]::[Test] :  [" << componentTagName.str() << "] : ..... FAILED" << std::endl
						<< "|- std::exception catch : [" << ex.what() << "]" << std::endl
						<< std::endl
						;
					return false;
				}
				catch (...)
				{
					std::cerr
						<< "[Error]::[Test] : [" << componentTagName.str() << "] : ..... FAILED" << std::endl
						<< "|- Unknown type catch" << std::endl
						<< std::endl
						;
					return false;
				}
			}
		};
		template <typename ...T_ComponentsTest>
		static void	TestMultipleComponents(void)
		{
			GCL::TMP::Foreach<T_ComponentsTest...>::Call<TestOneComponent>();
			std::cout << std::endl << "[Test] : Success Rate : " << callCount << " / " << sizeof...(T_ComponentsTest) << std::endl;
		}
	};
	uint32_t Test::callCount = 0;
}

int	main(int ac, char* av[])
{
	Tool::Test::TestMultipleComponents
	<
		GCL::Serialization::Test
		, GCL::Experimental::TypeTrait::Test
		, GCL::TMP::Test
		, GCL::TypeTrait::Test
		, GCL::Introspection::Test
		
		//, GCL::Task::Test
		//, GCL::Events::Test
		//, GCL::Container::Test
		//, GCL::Ownership::Test

		, GCL::Experimental::TypeTrait::Test
		// GCL::Experimental::Puzzle::Test
		// , GCL::Color::Test // FIXME
		, GCL::Experimental::Pattern::Test
		, GCL::Experimental::test_utils::Inline::Test
	>();

	system("pause");
	return 0;
}