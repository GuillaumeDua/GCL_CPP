#pragma once

#include <gcl_cpp/mp.hpp>
#include <type_traits>

namespace gcl::test
{
	struct mp
	{
		struct super
		{
			struct A {}; struct B {}; struct C {};
			constexpr static void proceed()
			{
				using type = gcl::mp::super<A, B, C>;
				static_assert(std::is_base_of_v<A, type>);
				static_assert(std::is_base_of_v<B, type>);
				static_assert(std::is_base_of_v<C, type>);
			}
		};
		struct partial_template
		{
			struct type
			{
				constexpr static void proceed()
				{
					using is_same = gcl::mp::partial_template<std::is_same>::type<bool, bool>;
					static_assert(is_same::value);

					using is_same_with_first_parameter = gcl::mp::partial_template<std::is_same, bool>::type<bool>;
					static_assert(is_same_with_first_parameter::value);
				}
			};
			struct value
			{
				constexpr static void proceed()
				{
					static_assert(gcl::mp::partial_template<std::is_same, bool>::value<bool>);
				}
			};

			using dependencies_t = std::tuple<type, value>;
		};
		struct require
		{
			template <typename T>
			struct my_constraint
			{
				constexpr static bool value =
					std::is_constructible_v<T, int> &&
					not std::is_pointer_v<T>
					;
			};

			struct on
			{
				static constexpr void proceed()
				{
					gcl::mp::require
					<
						my_constraint,
						gcl::mp::partial_template<std::is_same, long>::type,
						gcl::mp::partial_template<std::is_integral>::type
					>::on<long>();
				}
			};
			struct values_on
			{
				static constexpr void proceed()
				{
					constexpr auto result = gcl::mp::require
					<
						gcl::mp::partial_template<std::is_same, double>::type,
						gcl::mp::partial_template<std::is_same, bool>::type
					>::values_on<bool>;

					static_assert(result[0] == false);
					static_assert(result[1] == true);
				}
			};
			using dependencies_t = std::tuple<on, values_on>;
		};

		using dependencies_t = std::tuple
		<
			super,
			partial_template,
			require
		>;
	};
}