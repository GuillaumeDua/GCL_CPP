#pragma once

# include <gcl_cpp/test.hpp>
# include <gcl_cpp/container/polymorphic_reference.hpp>

namespace gcl::test::container_partial_impl
{
	struct polymorphic_reference
	{
		static void implicit_cast_and_operator_equal()
		{
			int i = 41;
			gcl::container::polymorphic_reference ref{ i };
			i += 1;
			GCL_TEST__EXPECT_VALUE(i, 42);
			ref = 13;
			GCL_TEST__EXPECT_VALUE(i, 13);

			gcl::container::polymorphic_reference ref2 = ref;
			ref2 = 2;
			GCL_TEST__EXPECT_VALUE(i, 2);
			ref = 3;
			GCL_TEST__EXPECT_VALUE(i, 3);

			GCL_TEST__EXPECT_EXCEPTION(std::bad_cast, [&ref]()
			{
				ref = std::string{ "hello there" };
			});
		}

		static void invoke_and_operator_parenthesis()
		{
			using func_type = std::function<void()>;
			int call_count{ 0 };
			func_type f = [&call_count]() { ++call_count; };

			gcl::container::polymorphic_reference func_ref{ f };
			func_ref.operator()<func_type>();
			GCL_TEST__EXPECT_VALUE(call_count, 1);
			func_ref.invoke<func_type>();
			GCL_TEST__EXPECT_VALUE(call_count, 2);
			func_ref = std::move(func_type{ std::move([&call_count]() { call_count += 2; }) });
			func_ref.invoke<func_type>();
			GCL_TEST__EXPECT_VALUE(call_count, 4);
			f();
			GCL_TEST__EXPECT_VALUE(call_count, 6);
		}

		static void proceed()
		{
			implicit_cast_and_operator_equal();
			invoke_and_operator_parenthesis();
		}
	};
}