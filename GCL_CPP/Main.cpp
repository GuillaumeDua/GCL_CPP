# include "Task.h"
# include "Maths.h"

# include <chrono>
# include <thread>
# include <iomanip>
# include <typeinfo>

template <typename T_ComponentTest>
bool	TestComponent(void)
{
	try
	{
		bool ret = T_ComponentTest::Proceed();
		std::cout << "[Test] : [" << typeid(T_ComponentTest).name() << "] : ..... " << (ret ? "PASSED" : "FAILED") << std::endl;
		return ret;
	}
	catch (const std::exception & ex)
	{
		std::cerr
			<< "[Error]::[Test] :  [" << typeid(T_ComponentTest).name() << "] : ..... FAILED" << std::endl
			<< "|- std::exception catch : [" << ex.what() << "]" << std::endl
			;
		return false;
	}
	catch (...)
	{
		std::cerr
			<< "[Error]::[Test] : [" << typeid(T_ComponentTest).name() << "] : ..... FAILED" << std::endl
			<< "|- Unknown type catch" << std::endl
			;
		return false;
	}
}

int	main(int ac, char* av[])
{
	// TestComponent<GCL::Task::Test>();
	TestComponent<GCL::Maths::Test>();

	system("pause");
	return 0;
}