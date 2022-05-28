#pragma once

#include <gcl_cpp/test.hpp>
#include <gcl_cpp/container/component_aggregator.hpp>

namespace gcl::test::container_impl
{
	struct position_type
	{
		uint32_t x, y;
		bool operator==(const position_type & other) const
		{
			return x == other.x && y == other.y;
		}
		bool operator!=(const position_type & other) const
		{
			return not (*this == other);
		}
	};
	struct size_type
	{
		uint32_t width, height;
	};

	namespace component
	{
		template <typename T>
		struct mover
		{
			position_type position;
			void move(const position_type & pos)
			{
				static_cast<T&>(*this).position = pos;
			}
		};
		template <typename T>
		struct resizer
		{
			size_type size;
			void resize(const size_type & size)
			{
				static_cast<T&>(*this).size = size;
			}
		};
		template <typename T>
		struct rendering
		{
			void render()
			{
			}
		};
	}

	struct component_aggregator
	{
		template
		<
			template <typename> typename ... components
		>
		using aggregator_type = gcl::container::component_aggregator::type<components...>;

		using entity_type_1 = aggregator_type<component::mover, component::resizer>;
		using entity_type_2 = aggregator_type<component::mover, component::rendering>;
		using entity_type_3 = aggregator_type<component::resizer, component::rendering>;

		struct type
		{
			static void proceed()
			{
				entity_type_1 toto;
				toto.as_component<component::mover>().move({ 42, 42 });
				GCL_TEST__EXPECT_VALUE(toto.position, position_type{ 42,42 });
			}
		};

		struct wrapper
		{
			static void proceed()
			{
				gcl::container::component_aggregator::wrapper wrapper_1{ std::move(entity_type_1{}) };

				GCL_TEST__EXPECT(wrapper_1.has_component<component::mover>());
				GCL_TEST__EXPECT(wrapper_1.has_component<component::resizer>());
				GCL_TEST__EXPECT(not wrapper_1.has_component<component::rendering>());
			}
		};

		using dependencies_t = std::tuple<type, wrapper>;
	};
}