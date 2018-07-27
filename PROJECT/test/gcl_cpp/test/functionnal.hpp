#pragma once

#include <gcl_cpp/functionnal.hpp>
#include <gcl_cpp/test.hpp>

namespace gcl::test
{
	struct functionnal
	{
		struct trait
		{
			static void proceed()
			{
				auto lambda = []() {};
				using func_with_parameter_type = std::function<int(std::string &&)>;
				
				GCL_TEST__EXPECT(std::is_same_v<gcl::functionnal::trait<decltype(lambda)>::return_type, void>);
				GCL_TEST__EXPECT(std::is_same_v<gcl::functionnal::trait<func_with_parameter_type>::arguments_type::type_at<0>, std::string&&>);
			}
		};

		struct combine
		{
			static void proceed()
			{
				using func_t = std::function<void(std::string &)>;

				struct toto
				{
					toto() = default;
					toto(const toto &) = default; // delete; -> todo : bad
					toto(toto &&) = default;

					void a(std::string & str) const
					{
						str += "d";
					}
				} toto_instance;

				auto ret = gcl::functionnal::combine
				(
					func_t{ [](std::string & str) { str += "a"; } },
					func_t{ [](std::string & str) { str += "b"; } },
					func_t{ [](std::string & str) { str += "c"; } },
					std::bind(&toto::a, toto_instance, std::placeholders::_1)
				);
				std::string str;
				ret(str);

				GCL_TEST__EXPECT_VALUE(str, "abcd");

				auto empty_func = gcl::functionnal::combine();
			}
		};

		struct callable_operator_plus
		{
			static void proceed()
			{
				int i = 0;
				auto merged_lambdas = std::function{ [&i]() { ++i; } } +std::function{ [&i]() { ++i; } };
				GCL_TEST__EXPECT_VALUE(i, 2);
			}
		};

		using dependencies_t = gcl::type_info::pack<trait, combine>;
	};
}