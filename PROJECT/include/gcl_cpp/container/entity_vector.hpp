#pragma once

// stand-alone POC : https://godbolt.org/g/2VHRHE

#include <gcl_cpp/container/polymorphic_vector.hpp>
#include <gcl_cpp/introspection.hpp>

GCL_Introspection_Generate__has_nested(properties_t)

namespace gcl
{
	namespace container
	{
		struct entity_vector : private gcl::container::polymorphic_vector
		{
			using base_t = polymorphic_vector;

			using base_t::get;
			using base_t::visit;
			//using base_t::remove_if;

			template <typename ... values_t>
			entity_vector(values_t && ... values)
			{
				(push_back(std::forward<values_t>(values)), ...);
			}

			template
			<
				typename T,
				typename std::enable_if_t<gcl::introspection::generated::has_nested_type::properties_t<T>::value>* = nullptr
			>
			void push_back(T && value)
			{
				base_t::push_back(std::forward<decltype(value)>(value));
				register_properties(get().back(), T::properties_t{});
			}
			template
			<
				typename T,
				typename std::enable_if_t<!gcl::introspection::generated::has_nested_type::properties_t<T>::value>* = nullptr
			>
			void push_back(T && value)
			{
				base_t::push_back(std::forward<decltype(value)>(value));
			}


		private:
			template
			<
				template <typename ...> class properties_pack_t,
				typename ... properties_ts
			>
			void register_properties(const value_type_ptr & value, properties_pack_t<properties_ts...>)
			{
				(void)std::initializer_list<int>{ ((void)register_type<properties_ts>(value), 0)... };
			}
		};
	}
	namespace deprecated::container
	{
		template <typename T>
		struct entity_vector : public polymorphic_vector<T>
		{
			using value_type = T;
			using base_t = polymorphic_vector<T>;

			using base_t::get;
			using base_t::visit;
			using base_t::remove_if;

			template
			<
				typename entity_t,
				typename std::enable_if_t<gcl::introspection::generated::has_nested_type::properties_t<entity_t>::value>* = nullptr
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
				typename std::enable_if_t<!gcl::introspection::generated::has_nested_type::properties_t<entity_t>::value>* = nullptr
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
				(void)std::initializer_list<int>{ ((void)push_back_property<properties_ts>(value), 0)... };
			}
			template <typename property_t>
			void push_back_property(value_type * value)
			{
				content_sorted_accessor[gcl::deprecated::type_info::id<property_t>::value].push_back(value);
			}
		};
	}
}
