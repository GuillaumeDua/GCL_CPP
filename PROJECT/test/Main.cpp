// #define _GCL_DEBUG

// #include <gcl/maths.h>
// #include <gcl/mp/mp.hpp>
// #include <gcl/serialization.hpp>
// 
// #include <gcl/test/gcl.hpp>
// #include <gcl/signals.hpp>

#include <stdexcept>
#include <iostream>

auto main(int, char const * []) -> int
{
	try
	{
		//gcl::signals::initialize();
		// gcl::test::proceed();
	}
	catch (const std::exception & ex)
	{
		std::cerr << "Error : " << ex.what() << std::endl;
	}
	catch (...)
	{
		std::cerr << "Error : unknown" << std::endl;
	}

	return 0;
}