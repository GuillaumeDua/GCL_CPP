#pragma once // not using standard header-guard in order to not pollute macro completion on GCL_TEST*

# include <gcl_cpp/container/entity_vector.hpp>
# include <gcl_cpp/test.hpp>

namespace gcl::test::container_impl
{
	struct entity_vector
	{
		struct properties
		{
			struct is_drawable {};
			struct is_controlable {};
		};
		struct types
		{
			struct A
			{
				using properties_t = std::tuple<properties::is_drawable, properties::is_controlable>;
			};
			struct B
			{
				using properties_t = std::tuple<properties::is_drawable>;
			};
			struct C
			{
				using properties_t = std::tuple<properties::is_controlable>;
			};
			struct D {};
		};

		static auto generate_container()
		{
			return gcl::container::entity_vector
			{
				types::A{},
				types::B{},
				types::C{},
				types::D{},
				types::A{},
				types::B{},
				types::C{},
				types::D{}
			};
		}

		struct create
		{
			static void proceed()
			{
				gcl::container::entity_vector entities = generate_container();

				GCL_TEST__EXPECT_VALUE(entities.get().size(), 8);
				GCL_TEST__EXPECT_VALUE(entities.get<types::A>().size(), 2);
				GCL_TEST__EXPECT_VALUE(entities.get<types::B>().size(), 2);
				GCL_TEST__EXPECT_VALUE(entities.get<types::C>().size(), 2);
				GCL_TEST__EXPECT_VALUE(entities.get<types::D>().size(), 2);

				GCL_TEST__EXPECT_VALUE(entities.get<properties::is_drawable>().size(), 4);
				GCL_TEST__EXPECT_VALUE(entities.get<properties::is_controlable>().size(), 4);
			}
		};

		struct remove
		{
			struct remove_by_type
			{
				static void proceed()
				{
					gcl::container::entity_vector entities = generate_container();

					entities.remove<types::A>();

					GCL_TEST__EXPECT_VALUE(entities.get().size(), 6);
					GCL_TEST__EXPECT_VALUE(entities.get<types::A>().size(), 0);
					GCL_TEST__EXPECT_VALUE(entities.get<types::B>().size(), 2);
					GCL_TEST__EXPECT_VALUE(entities.get<types::C>().size(), 2);
					GCL_TEST__EXPECT_VALUE(entities.get<types::D>().size(), 2);

					GCL_TEST__EXPECT_VALUE(entities.get<properties::is_drawable>().size(), 2);
					GCL_TEST__EXPECT_VALUE(entities.get<properties::is_controlable>().size(), 2);
				}
			};
			struct remove_by_property
			{
				static void proceed()
				{
					gcl::container::entity_vector entities = generate_container();

					entities.remove<properties::is_drawable>();

					GCL_TEST__EXPECT_VALUE(entities.get().size(), 4);
					GCL_TEST__EXPECT_VALUE(entities.get<types::A>().size(), 0);
					GCL_TEST__EXPECT_VALUE(entities.get<types::B>().size(), 0);
					GCL_TEST__EXPECT_VALUE(entities.get<types::C>().size(), 2);
					GCL_TEST__EXPECT_VALUE(entities.get<types::D>().size(), 2);

					GCL_TEST__EXPECT_VALUE(entities.get<properties::is_drawable>().size(), 0);
					GCL_TEST__EXPECT_VALUE(entities.get<properties::is_controlable>().size(), 2);
				}
			};

			using dependencies_t = std::tuple<remove_by_type, remove_by_property>;
		};

		using dependencies_t = std::tuple<create, remove>;
	};
}

namespace gcl::test::deprecated::container_impl
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
			using entity_container_t = gcl::deprecated::container::entity_vector<entity>;
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
				toto(std::string &&) {}
			};
			container.emplace_back<toto>("hello");
		}
	};
}