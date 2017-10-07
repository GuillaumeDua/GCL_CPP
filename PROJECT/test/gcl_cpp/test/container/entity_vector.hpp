#pragma once // not using standard header-guard in order to not pollute macro completion on GCL_TEST*

# include <gcl_cpp/container/entity_vector.hpp>
# include <gcl_cpp/test.hpp>

namespace gcl
{
	namespace test
	{
		namespace container_partial_impl
		{
			struct entity_vector
			{
				struct entity_properties
				{
					struct collision {};
					struct control {};
					struct garbage {};
					struct unused_property {};
				};

				struct entity {};
				struct entity_A : entity {};
				struct entity_B : entity
				{
					using properties_t = std::tuple
					<
						entity_properties::collision,
						entity_properties::control,
						entity_properties::garbage
					>;
				};
				struct entity_C : entity
				{
					using properties_t = std::tuple<entity_properties::control>;
				};
				struct entity_D : entity
				{
					using properties_t = std::tuple<entity_properties::collision>;
				};

				static void proceed()
				{
					using entity_container_t = gcl::container::entity_vector<entity>;
					entity_container_t container;

					container.push_back(std::make_unique<entity_A>());	// no property
					container.push_back(std::make_unique<entity_A>());	// no property
					container.push_back(std::make_unique<entity_B>());	// collision, control, garbage
					container.push_back(std::make_unique<entity_C>());	// control
					container.push_back(std::make_unique<entity_D>());	// collision

					GCL_TEST__EXPECT_VALUE(container.get<entity_A>().size(), std::size_t{ 2 });
					GCL_TEST__EXPECT_VALUE(container.get<entity_properties::collision>().size(), std::size_t{ 2 });

					GCL_TEST__EXPECT_EXCEPTION(std::out_of_range, [&]() {container.get<entity_properties::unused_property>(); });

					struct toto : entity
					{
						using properties_t = std::tuple
							<
							entity_properties::collision,
							entity_properties::control,
							entity_properties::garbage
							>;
						toto(std::string && str) {}
					};
					container.emplace_back<toto>("hello");
				}
			};
		};

	}
}