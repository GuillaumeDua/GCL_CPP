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
		struct meet_requirement
		{
			static constexpr void proceed()
			{
				struct A { A() = delete; };
				static_assert(not gcl::mp::meet_requirement<A, std::is_default_constructible>);
				static_assert(gcl::mp::meet_requirement<int, std::is_pod, std::is_default_constructible>);
			}
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
		struct is_unique
		{
			static constexpr void proceed()
			{
				constexpr bool yes = gcl::mp::is_unique_v
				<
					int,
					double, int, char, float
				>;
				static_assert(yes);
				constexpr bool no = gcl::mp::is_unique_v
				<
					int,
					double, int, char, float, int
				>;
				static_assert(not no);
			}
		};
		struct make_reverse_index_sequence
		{
			constexpr static void proceed()
			{
				static_assert(std::is_same_v
				<
					gcl::mp::make_reverse_index_sequence<5>,
					std::index_sequence<4,3,2,1,0>
				>);
			}
		};
		struct reverse_variadic_order
		{
			constexpr static void proceed()
			{
				using reverse_variadic_type = gcl::mp::reverse_variadic_order_t<int, double, char, std::string>;
				static_assert(std::is_same_v
				<
					reverse_variadic_type,
					gcl::mp::type_pack<std::string, char, double, int>
				>);

				using ordered_types = gcl::mp::type_pack<int, double, char, float, std::string>;
				static_assert(std::is_same_v
				<
					ordered_types,
					std::decay_t<decltype(gcl::mp::reverse_variadic_order(gcl::mp::reverse_variadic_order(ordered_types{})))>
				>);
			}
		};
		struct filter
		{
			struct as_bitset
			{
				static void proceed()
				{
					constexpr auto filter = gcl::mp::filter::as_bitset
					(
						gcl::mp::type_pack<int, double, float, std::string>{},
						gcl::mp::type_pack<float, std::string>{}
					);
					static_assert(std::is_same_v
					<
						std::decay_t<decltype(filter)>,
						std::bitset<4>
					>);
					GCL_TEST__EXPECT_VALUE(filter, std::decay_t<decltype(filter)>{"1100"}); // bitset are reverse bit ordered
				}
			};
			using dependencies_t = gcl::mp::type_pack<as_bitset>;
		};

		using dependencies_t = std::tuple
		<
			super,
			meet_requirement,
			partial_template,
			require,
			is_unique,
			make_reverse_index_sequence,
			reverse_variadic_order,
			filter
		>;
	};
}