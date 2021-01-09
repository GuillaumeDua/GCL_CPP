#pragma once // not using standard header-guard in order to not pollute macro completion on GCL_TEST*

# include <gcl_cpp/test.hpp>
# include <tuple>

namespace gcl
{
	namespace test
	{
		struct test // test test
		{
			struct check
			{
				static void proceed()
				{
					GCL_TEST__EXPECT(1 == 1);
					GCL_TEST__EXPECT_VALUE(1, 1);
					int i = 1;
					GCL_TEST__EXPECT_VALUES(i, 1, [&i]() {++i; }, 2);

					GCL_TEST__EXPECT_VALUE_IN_RANGE(42, gcl::test::check::value_range_t<int>{ 0, 100 });
				}
			};

			using dependencies_t = std::tuple<check>;
		};
	}
}
