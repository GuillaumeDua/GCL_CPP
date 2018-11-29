#pragma once // not using standard header-guard in order to not pollute macro completion on GCL_TEST*

#include <gcl_cpp/test/event.hpp>
#include <gcl_cpp/test/test.hpp>
#include <gcl_cpp/test/type_info.hpp>
#include <gcl_cpp/test/type_index.hpp>
#include <gcl_cpp/test/container.hpp>
#include <gcl_cpp/test/pattern.hpp>
#include <gcl_cpp/test/functionnal.hpp>
#include <gcl_cpp/test/introspection.hpp>
#include <gcl_cpp/test/mp.hpp>

#include <gcl_cpp/test.hpp>

namespace gcl
{
	namespace test
	{
		static void proceed()
		{
			::gcl::test::components
			<
				::gcl::test::test,
				::gcl::test::event,
				::gcl::test::type_info,
				::gcl::test::type_index,
				::gcl::test::container,
				::gcl::test::pattern,
				::gcl::test::functionnal,
				::gcl::test::introspection,
				::gcl::test::mp
			>::test();

			::gcl::test::components
			<	// deprecated components
				::gcl::test::deprecated::type_info,
				::gcl::test::deprecated::container
			>::test();
		}
	}
}
