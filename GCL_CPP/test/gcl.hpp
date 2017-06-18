#ifndef GCL_TEST_GCL_HPP__
# define GCL_TEST_GCL_HPP__

#include <gcl_cpp/test/event.hpp>
#include <gcl_cpp/test/test.hpp>
#include <gcl_cpp/test/container/polymorphic_vector.hpp> // todo
#include <gcl_cpp/test/type_info.hpp> // todo

#include <gcl_cpp/test.hpp>

namespace gcl
{
	namespace test
	{
		static void proceed()
		{
			// ::gcl::test::component<::gcl::test::test>::test();
			// ::gcl::test::component<::gcl::test::event>::test();
			::gcl::test::components
			<
				::gcl::test::test,
				::gcl::test::event
			>::test();
		}
	}
}

#endif // GCL_TEST_GCL_HPP__