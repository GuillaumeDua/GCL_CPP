// #define _GCL_DEBUG

# include "Task.h"
# include "Maths.h"
# include "EventHandler.h"
# include "SelfUpdate.h"
# include "Ownership.h"
# include "TypeTraits.h"
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
		namespace TestUtils
		{
			std::mutex									Inline::RT_scope_controler::_container_mutex;
			std::mutex									Inline::RT_scope_controler::_obs_mutex;
			volatile bool								Inline::RT_scope_controler::_isActive;
			Inline::RT_scope_controler::T_Container		Inline::RT_scope_controler::_container;
			Inline::RT_scope_controler::T_ObserversList	Inline::RT_scope_controler::_observers;
		}
	}
}

# include <chrono>
# include <thread>
# include <iomanip>
# include <typeinfo>
# include <sstream>

namespace Tool
{
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
		
		//, GCL::Task::Test
		//, GCL::Events::Test
		//, GCL::Container::Test
		//, GCL::Ownership::Test

		, GCL::Experimental::TypeTrait::Test
		// GCL::Experimental::Puzzle::Test
		// , GCL::Color::Test // FIXME
		, GCL::Experimental::Pattern::Test
		, GCL::Experimental::TestUtils::Inline::Test
	>();

	system("pause");
	return 0;
}