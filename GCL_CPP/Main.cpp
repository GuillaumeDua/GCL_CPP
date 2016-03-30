// #define _GCL_DEBUG

# include "Task.h"
# include "Maths.h"
# include "Notification.h"
# include "SelfUpdate.h"
# include "Ownership.h"

# include <chrono>
# include <thread>
# include <iomanip>
# include <typeinfo>

template <typename T_ComponentTest>
bool	TestComponent(void)
{
	try
	{
		const std::string & symbol_name = typeid(T_ComponentTest).name();
		std::cout << "[Test] : [" << std::setw(30) << symbol_name << "] : Processing ... " << std::endl;

		std::chrono::high_resolution_clock::time_point tp_start = std::chrono::high_resolution_clock::now();
		bool ret = T_ComponentTest::Proceed();
		const long long elasped_usec = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - tp_start).count();

		std::cout << "[Test] : [" << std::setw(30) << symbol_name << "] : .............. : [" << (ret ? "PASSED" : "FAILED") << "] in " << elasped_usec  << "ms" << std::endl << std::endl;

		return ret;
	}
	catch (const std::exception & ex)
	{
		std::cerr
			<< "[Error]::[Test] :  [" << std::setw(30) << typeid(T_ComponentTest).name() << "] : ..... FAILED" << std::endl
			<< "|- std::exception catch : [" << ex.what() << "]" << std::endl
			<< std::endl
			;
		return false;
	}
	catch (...)
	{
		std::cerr
			<< "[Error]::[Test] : [" << std::setw(30) << typeid(T_ComponentTest).name() << "] : ..... FAILED" << std::endl
			<< "|- Unknown type catch" << std::endl
			<< std::endl
			;
		return false;
	}
}

int	main(int ac, char* av[])
{
	// TestComponent<GCL::Task::Test>();
	// TestComponent<GCL::Maths::Test>();
	TestComponent<GCL::Notification::Test>();
	TestComponent<GCL::Container::Test>();
	TestComponent<GCL::Ownership::Test>();

	system("pause");
	return 0;
}