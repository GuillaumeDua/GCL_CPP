#pragma once

// info :
// define preprocessor gcl_ecs_enable_output to enable std::ostream & operator<< <ecs::<type_name>>(std::ostream &, const ecs::<type_name> &)

#include <gcl_cpp/type_info.hpp>
#include <gcl_cpp/type_traits.hpp>
#if defined (gcl_ecs_enable_output)
# include <gcl_cpp/IO.h> // default std::ostream & operator<< <T>(std::ostream &, const T &)
#endif

#include <vector>
#include <iostream>
#include <iomanip>
#include <functional>
#include <tuple>
#include <type_traits>
#include <bitset>

// std::conjunction_v instead of fold expression, as it is MVSC 15.9.1.
// fold expression, including in constexpr contexts (static_assert) will be fix in 16.0
// https://developercommunity.visualstudio.com/content/problem/331686/c-fold-expression-in-static-assert-does-not-compil.html
// e.g : constexpr bool b = true; static_assert(b); // does not compile

namespace gcl::pattern::ecs
{
	using id_type = std::size_t;

	template <typename ... ts_components>
	struct components
	{	// MP helper for component types

		static_assert(gcl::mp::are_unique<ts_components...>);

		using mp_variadic_helper = gcl::type_info::variadic_template<ts_components...>;

		static constexpr inline auto count = mp_variadic_helper::size;
		template <typename T>
		static constexpr inline auto index_of = mp_variadic_helper::template  index_of<T>;
		template <std::size_t index>
		using type_at = typename mp_variadic_helper::template type_at<index>;

		template <typename T>
		static constexpr inline bool contains = mp_variadic_helper::template contains<T>;

		using mask_type = std::bitset<sizeof...(ts_components)>;

		using as_type_pack = gcl::mp::type_pack<ts_components...>;
	};

	template <typename ... ts_components>
	struct entity
	{
		static_assert(gcl::mp::are_unique<ts_components...>);
		using components_type = components<ts_components...>;

		operator id_type() const
		{
			return id;
		}

		void reset(id_type new_id)
		{	// create a valid entry
			id = new_id;
			is_alive = false;
			components_mask.reset();
		}

		id_type id; // unique per entity, index in the storage.
		bool is_alive = false; // !to_garbage
		typename components_type::mask_type components_mask; // ts_components -> bools "is_active"
	};

	template <typename ... ts_components>
	struct components_storage
	{	// per-component contiguous storage

		static_assert(gcl::mp::are_unique<ts_components...>);
		//static_assert((gcl::mp::meet_requirement<ts_components, std::is_default_constructible> && ...)); // msvc 16.0
		//static_assert((std::is_default_constructible_v<ts_components> && ...));
		static_assert(std::conjunction_v<std::is_default_constructible<ts_components>...>);
		static_assert(not std::disjunction_v<std::is_reference<ts_components>...>); // refs are not default-constructible

		using components_type = components<ts_components...>;

		constexpr static inline auto count = components_type::count;
		
		components_storage()
		{}

		template <typename T>
		auto & get(std::size_t index)
		{
			return std::get<std::vector<T>>(content).at(index);
		}
		template <typename T>
		const auto & get(std::size_t index) const
		{
			return std::get<std::vector<T>>(content).at(index);
		}

		void resize(std::size_t size)
		{
			gcl::tuple_utils::for_each(content, [&size](auto & value)
			{
				value.resize(size);
			});
		}
		void reserve(std::size_t size)
		{
			gcl::tuple_utils::for_each(content, [&size](auto & value)
			{
				value.reserve(size);
			});
		}
		auto size() const
		{	// size is always the same of all vectors
			static_assert(count > 0);
			return std::get<0>(content).size();
		}
		auto capacity() const
		{	// size is always the same of all vectors
			static_assert(count > 0);
			return std::get<0>(content).capacity();
		}
		void clear()
		{
			gcl::tuple_utils::for_each(content, [](auto & value) { value.clear(); });
		}

		void swap_index(std::size_t lhs_index, std::size_t rhs_index)
		{
			gcl::tuple_utils::for_each(content, [&](auto & component_vector)
			{
				std::swap(component_vector.at(lhs_index), component_vector.at(rhs_index));
			});
		}

#if defined(gcl_ecs_enable_output)
		void print_to(std::ostream & os) const
		{
			for (std::size_t i{ 0 }; i < size(); ++i)
			{
				((os << '[' << std::setw(8) << get<ts_components>(i) << "] "), ...) << '\n';
			}
		}
#endif

	private:
		using underlying_type = std::tuple<std::vector<ts_components>...>;
		underlying_type content;
	};

	template <typename ... ts_components>
	struct contract
	{	// establish a contract.e.g a requierement

		static_assert(gcl::mp::are_unique<ts_components...>);
		using components_type = components<ts_components...>;
	};

	template <typename ... ts_components>
	struct manager
	{	// manage an entitiy collection
		// entities and components are synchronized by index

		static_assert(gcl::mp::are_unique<ts_components...>);

		using entity_type = typename ecs::entity<ts_components...>;
		using components_type = typename ecs::components<ts_components...>;
		using components_storage_type = typename ecs::components_storage<ts_components...>;

		manager(std::size_t initial_capacity)
		{
			reserve_capacity(std::max<decltype(capacity())>(initial_capacity, 1));
		}

		auto & entity_at(id_type id)
		{	// todo : throw std::out_of_range if :
			// id < entity_creation_index || not entities.at(id).is_alive ?
			
			return entities.at(id);
		}
		auto & entity_at(id_type id) const
		{
			return entities.at(id);
		}
		auto create_index()
		{	// create a new initialized entity
			// like std::vector, double capacity if needed

			assert(entity_creation_index <= size());
			assert(capacity() >= size());
			assert(capacity() > 0);

			if (capacity() == entity_creation_index)
				reserve_capacity(capacity() * 2);

			auto index{ entity_creation_index++ };
			auto & entity = entities.at(index);
			assert(not entity.is_alive);
			entity.components_mask.reset();
			entity.is_alive = true;
			return entity.id;
		}

		template <typename ... requiered_components>
		auto create_entity()
		{	// reference may point to the wrong element after reordering
			//static_assert((components_type::contains<requiered_components>> && ...)); // todo : fixed in MVSC 16.0

			auto & entity = entities.at(create_index());
			using return_type = std::tuple
			<
				entity_type&,
				std::tuple<requiered_components &...>
			>;
			return return_type
			{
				entity,
				std::tuple<requiered_components &...>{entity_add_component<requiered_components>(entity.id)...}
			};
		}
		template <typename ... requiered_components, typename ... requiered_components_args>
		auto create_entity(requiered_components_args && ... args)
		{
			auto & entity = entities.at(create_index());
			using return_type = std::tuple
			<
				entity_type&,
				std::tuple<requiered_components &...>
			>;
			return return_type
			{
				entity,
				std::tuple<requiered_components &...>{entity_add_component<requiered_components>(entity.id, std::forward<requiered_components_args>(args))...}
			};
			/*auto &[entity, comp] = create_entity<requiered_components...>();
			comp = std::forward_as_tuple(args...);

			using return_type = decltype(create_entity<requiered_components...>());
			return return_type{ entity, comp };*/
		}
		void destroy_entity(id_type id)
		{
			entity_at(id).is_alive = false;
		}

		auto entities_count() const
		{
			return entity_creation_index;
		}
		void clear() noexcept
		{
			for (auto index{ 0 }; index < capacity(); ++index)
				entities[index].reset(index);
			entity_creation_index = 0;
		}
		void reorder()
		{	// reorder (optimize) internal storage and indexes
			// warning : all previously acquiered reference to content (entities, components) may be invalide then.

			const auto entity_comparator = [](const entity_type & lhs_entity, const entity_type & rhs_entity)
			{
				return lhs_entity.is_alive && not rhs_entity.is_alive;
			};
			std::sort(std::begin(entities), std::end(entities), entity_comparator);

			for (std::size_t entity_it{ 0 }; entity_it < entity_creation_index; ++entity_it)
			{	// reorder alive entities
				auto & entity = entities[entity_it];
				if (not entity.is_alive)
				{	// update creation index
					entity_creation_index = entity_it;
					break;
				}
				components.swap_index(entity_it, entity.id);
				entity.id = entity_it;
			}
			for (std::size_t entity_it{ entity_creation_index }; entity_it < entities.size(); ++entity_it)
			{	// reorder dead entities
				auto & entity = entities[entity_it];
				entity.id = entity_it;
			}
		}

		template <typename component, typename ... component_args_types>
		auto & entity_add_component(id_type id, component_args_types && ... component_args)
		{
			static_assert(components_type::template contains<component>);
			static_assert
			(
				std::is_constructible_v<component, decltype(component_args)...> or
				gcl::type_traits::is_brace_constructible_v<component, decltype(component_args)...>
			);

			auto & entity = entities.at(id);
			entity.components_mask[components_type::template index_of<component>] = true;

			auto & component_value = components.get<component>(entity.id);
			if constexpr (std::is_constructible_v<component, decltype(component_args)...>)
			{
				new (&component_value) component( std::forward<component_args_types>(component_args)... );
			}
			else if constexpr (gcl::type_traits::is_brace_constructible_v<component, decltype(component_args)...>)
			{
				new (&component_value) component{ std::forward<component_args_types>(component_args)... };
			}
			else
			{
				static_assert(false);
			}

			// `in-place new` vs `move temporary`
			// clang 6.0 (O3) : 1.1 times faster
			// http://quick-bench.com/-SmOBKMORqiRQblURvp9O8vOpiU
			// GCC-8.2   (O3) : 1.4 times faster
			// http://quick-bench.com/_xs-64Ofc9l8Vn2fMRvTqCOYQNg
			// also, generate way less assembly
			// https://godbolt.org/z/-6KJWm
			return component_value;
		}
		template <typename component, typename ... component_args>
		void entity_remove_component(id_type id)
		{
			static_assert(components_type::template has_component<component>);
			entity_at(id).components_mask[components_type::template index_of<component>] = false;

			auto & component_value = components_type::template get<component>(entity.id);
			// clean component ?
		}

		template <typename component>
		auto entity_has_component(id_type id) const
		{
			static_assert(components_type::template contains<component>);
			return entities.at(id).components_mask[components_type::template index_of<component>];
		}
		template <typename ... requiered_components>
		bool entity_has_components(id_type id) const
		{
			//static_assert(components_type::contains<requiered_components> && ...); // todo : fix with msvc 16.0

			static constexpr auto filter = gcl::mp::filter::as_bitset
			(
				components_type::as_type_pack{},
				gcl::mp::type_pack<requiered_components...>{}
			);

			static_assert(std::is_same_v
			<
				std::decay_t<decltype(filter)>,
				std::decay_t<decltype(std::declval<entity_type>().components_mask)>
			>);

			return (filter & entity_at(id).components_mask) == filter;
		}

		template <typename component>
		auto & entity_get_component(id_type entity_id)
		{
			static_assert(components_type::template contains<component>);
			if (!entity_has_component<component>(entity_id))
				throw std::runtime_error("ecs::manager::entity_get_component : requested component does not exists");
			return components.get<component>(entity_id);
		}

		template <typename function_type>
		void for_each_entities(function_type func)
		{	// for each (alive) entities
			for (id_type index{ 0 }; index < entity_creation_index; ++index)
				func(entity_at(index));
		}
		template <typename function_type>
		void for_each_entities(function_type func) const
		{
			for (id_type index{ 0 }; index < entity_creation_index; ++index)
				func(entity_at(index));
		}
		template <typename function_type, typename ... requiered_components>
		void for_each_entities(contract<requiered_components...>, function_type func)
		{
			using contract_type = contract<requiered_components...>;

			for_each_entities([this, &func](auto & entity)
			{
				const bool match_requierement = entity_has_components<requiered_components...>(entity.id);
				if (match_requierement)
					(func(entity, components.get<requiered_components>(entity.id)...));
			});
		}
		template <typename function_type, typename ... requiered_components>
		void for_each_entities(contract<requiered_components...>, function_type func) const
		{
			using contract_type = contract<requiered_components...>;

			for_each_entities([this, &func](auto & entity)
			{
				const bool match_requierement = entity_has_components<requiered_components...>(entity.id);
				if (match_requierement)
					(func(entity, components.get<requiered_components>(entity.id)...));
			});
		}

		const auto & get_entities() const
		{
			return entities;
		}
		const auto & get_components() const
		{
			return components;
		}

	private:
		std::vector<entity_type> entities;
		components_storage_type components;
		std::size_t entity_creation_index{ 0 };

		auto size() const
		{
			static_assert(components_type::count != 0);
			assert(entities.size() == components.size());
			return entities.size();
		}
		auto capacity() const
		{
			assert(entities.capacity() == components.capacity());
			return entities.capacity();
		}
		void reserve_capacity(std::size_t new_capacity)
		{	// extend capacity
			// initialize entities

			const auto previous_capacity = capacity();
			assert(new_capacity >= previous_capacity); // avoid useless calls
			assert(entities.capacity() == components.capacity());
			entities.resize(new_capacity);
			components.resize(new_capacity);

			for (auto created_index{ previous_capacity }; created_index < new_capacity; ++created_index)
			{
				auto & entity{ entities[created_index] };
				entity.reset(created_index);
			}
		}
	};
}

#if defined(gcl_ecs_enable_output)
template <class CharT, class Traits, typename ... ts_components>
std::basic_ostream<CharT, Traits>& operator<<(std::basic_ostream<CharT, Traits> & os,
	const ecs::components_storage<ts_components...> & storage);

template <class CharT, class Traits, typename ... ts_components>
std::basic_ostream<CharT, Traits>& operator<<(std::basic_ostream<CharT, Traits> & os,
	const ecs::manager<ts_components...> & manager);

template <class CharT, class Traits, typename ... ts_components>
std::basic_ostream<CharT, Traits>& operator<<(std::basic_ostream<CharT, Traits> & os,
	const ecs::components_storage<ts_components...> & storage)
{
	storage.print_to(os);
	return os;
}

template <class CharT, class Traits, typename ... ts_components>
std::basic_ostream<CharT, Traits>& operator<<(std::basic_ostream<CharT, Traits> & os,
	const ecs::manager<ts_components...> & manager)
{
	os
		<< "manager::components :\n"
		<< manager.get_components() << '\n'
		<< "manager::entities :\n"
		;
	manager.for_each_entities([&os](const auto & entity)
	{
		os << entity.id << " -> " << entity.components_mask << " -> " << (entity.is_alive ? "alive" : "dead") << '\n';
	});
	return os;
}
#endif
