#pragma once

// stand-alone POC : https://godbolt.org/g/2VHRHE

#include <gcl_cpp/container/polymorphic_vector.hpp>

namespace gcl
{
	struct entity_helper
	{
		template< class, class = std::void_t<> >
		struct has_properties : std::false_type { };
		template< class T >
		struct has_properties<T, std::void_t<typename T::properties_t>> : std::true_type { };
	};

	namespace container
	{
		template <typename T>
		struct entity_vector : private polymorphic_vector<T>
		{
			using base_t = polymorphic_vector<T>;

			using base_t::get;
			using base_t::visit;
			using base_t::remove_if;

			template
			<
				typename entity_t,
				typename std::enable_if_t<entity_helper::has_properties<entity_t>::value>* = nullptr
			>
			void push_back(std::unique_ptr<entity_t> && value)
			{
				static_assert(std::is_base_of<value_type, entity_t>::value, "value_type is not base of entity_t");
				base_t::push_back(std::forward<decltype(value)>(value));
				push_back_properties(content.back().get(), entity_t::properties_t{});
			}
			template
			<
				typename entity_t,
				typename std::enable_if_t<!entity_helper::has_properties<entity_t>::value>* = nullptr
			>
			void push_back(std::unique_ptr<entity_t> && value)
			{
				base_t::push_back(std::forward<decltype(value)>(value));
			}

			template <typename entity_t, typename ... Args>
			entity_t & emplace_back(Args&&... args)
			{
				static_assert(std::is_base_of<value_type, entity_t>::value, "value_type is not base of entity_t");
				auto elem{ std::make_unique<entity_t>(std::forward<Args>(args)...) };
				push_back<entity_t>(std::move(elem));
				return static_cast<entity_t&>(*(content.back().get()));
			}

		private:
			template
			<
				template <typename ...> class properties_pack_t,
				typename ... properties_ts
			>
			void push_back_properties(value_type * value, properties_pack_t<properties_ts...>)
			{
				int _[]{ (push_back_property<properties_ts>(value), 0)... };
			}
			template <typename property_t>
			void push_back_property(value_type * value)
			{
				content_sorted_accessor[gcl::type_info::id<property_t>::value].push_back(value);
			}
		};
	}
}

namespace entity_properties
{
	struct collision {};
	struct control {};
	struct garbage {};
}
using namespace entity_properties;

struct entity {};
struct entity_A : entity {};
struct entity_B : entity
{
	using properties_t = std::tuple<collision, control, garbage>;
};
struct entity_C : entity
{
	using properties_t = std::tuple<control>;
};
struct entity_D : entity
{
	using properties_t = std::tuple<collision>;
};

#include <iostream>
#include <iomanip>

void test_entity_vector()
{
	using entity_container_t = gcl::container::entity_vector<entity>;

	try
	{
		entity_container_t container;
		container.push_back(std::make_unique<entity_A>());	// no property
		container.push_back(std::make_unique<entity_A>());	// no property
		container.push_back(std::make_unique<entity_B>());	// collision, control, garbage
		container.push_back(std::make_unique<entity_C>());	// control
		container.push_back(std::make_unique<entity_D>());	// collision

		std::cout << "entity_A :\n";
		for (const auto & entity : container.get<entity_A>())
		{
			std::cout << '\t' << entity << std::endl;
		}

		std::cout << "entity_properties::collision :\n";
		for (const auto & entity : container.get<entity_properties::collision>())
		{
			std::cout << '\t' << entity << std::endl;
		}
	}
	catch (const std::exception & ex)
	{
		std::cerr << "exception : " << ex.what() << std::endl;
	}
	std::cout << std::endl;
}