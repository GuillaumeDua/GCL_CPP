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

# include <chrono>
# include <thread>
# include <iomanip>
# include <typeinfo>
# include <sstream>

template <typename T_ComponentTest>
bool	TestComponent(void)
{
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

		std::cout << "[Test] : [" << componentTagName.str() << "] : .............. : [" << (ret ? "PASSED" : "FAILED") << "] in " << elasped_usec  << "ms" << std::endl << std::endl;

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

int	main(int ac, char* av[])
{
	TestComponent<GCL::Experimental::Serialization::Test>();
	TestComponent<GCL::Experimental::TypeTrait::Test>();
	TestComponent<GCL::Experimental::Inheritance::Test>();
	// TestComponent<GCL::Experimental::Puzzle::Test>();
	TestComponent<GCL::Experimental::Puzzle3::Test>();

    // TestComponent<GCL::TMP::Test>();

    //TestComponent<GCL::Color::Test>();
        
    TestComponent<GCL::TypeTrait::Test>();
	

	// TestComponent<GCL::Events::Test>();

	// TestComponent<GCL::Task::Test>();
	// TestComponent<GCL::Maths::Test>();

	/*TestComponent<GCL::Events::Test>();
	TestComponent<GCL::Container::Test>();
	TestComponent<GCL::Ownership::Test>();
*/
	system("pause");
	return 0;
}