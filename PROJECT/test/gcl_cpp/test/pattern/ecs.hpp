#pragma once

#include <gcl_cpp/pattern/ecs.hpp>

namespace gcl::test::pattern_impl
{
	struct ecs
	{
		struct components
		{
			static void proceed()
			{
				using components_type = gcl::pattern::ecs::components<int, double, bool>;
				static_assert(components_type::count == 3);
				assert(components_type::index_of<double> == 1);
				static_assert(components_type::contains<double>);
				static_assert(std::is_same_v<double, components_type::type_at<1>>);
			}
		};
		struct components_storage
		{
			static void proceed()
			{
				using storage_type = gcl::pattern::ecs::components_storage<std::string, int, double>;
				static_assert(storage_type::count == 3);
				static_assert(std::is_same_v<int, storage_type::components_type::type_at<1>>);

				storage_type storage;

				storage.resize(42);
				assert(storage.size() == 42);

				storage.get<int>(1) = 13;
				storage.get<int>(2) = 42;
				storage.swap_index(1, 2);
				assert(storage.get<int>(1) == 42);
			}
		};
		struct manager
		{
			static void proceed()
			{
				using manager_type = gcl::pattern::ecs::manager<int, double, std::string>;
				manager_type manager{ 5 };

				{	// 0
					auto [value, comp] = manager.create_entity<>();
					value.is_alive = false;
				}
				{	// 1
					auto [value, comp] = manager.create_entity<int>();
					std::get<0>(comp) = 11;
					GCL_TEST__EXPECT_VALUE(value.id, 1);
					auto str_component = manager.entity_add_component<std::string>(value.id, std::size_t{ 5 }, '1');
					GCL_TEST__EXPECT_VALUE(str_component, std::string(5, '1'));
				}
				{	// 2
					auto [value, comp] = manager.create_entity<>();
					manager.entity_add_component<int>(value.id, 22);
					manager.destroy_entity(value);
				}
				{	// 3
					auto[entity_id, entity_persistent_id] = manager.create_index();
					manager.entity_add_component<int>(entity_id, 33);
				}
				{	// 4
					auto [value, comp] = manager.create_entity<double, int, std::string>(444, 44, "44444");
				}

				manager.reorder();

				std::vector<std::reference_wrapper<manager_type::entity_type>> matched;
				using contract_type = gcl::pattern::ecs::contract<std::string, int>;
				manager.for_each_entities
				(
					contract_type{},
					[&matched](auto & entity, std::string & str, int & i)
					{
						matched.push_back(entity);
					}
				);
				GCL_TEST__EXPECT_VALUE(matched.size(), 2);
				{
					manager_type::entity_type & entity_matched = matched.at(0);
					GCL_TEST__EXPECT(entity_matched.is_alive);
					GCL_TEST__EXPECT_VALUE(manager.entity_get_component<int>(entity_matched), 11);
					GCL_TEST__EXPECT_VALUE(manager.entity_get_component<std::string>(entity_matched), "11111");
				}
				{
					manager_type::entity_type & entity_matched = matched.at(1);
					GCL_TEST__EXPECT(entity_matched.is_alive);
					GCL_TEST__EXPECT_VALUE(manager.entity_get_component<int>(entity_matched), 44);
					GCL_TEST__EXPECT_VALUE(manager.entity_get_component<std::string>(entity_matched), "44444");
				}
			}
		};

		using dependencies_t = gcl::mp::type_pack
		<
			components,
			components_storage,
			manager
		>;
	};
}
