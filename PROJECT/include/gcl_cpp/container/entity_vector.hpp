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

	namespace container::deprecated
	{
		template <typename T>
		struct entity_vector : public deprecated::polymorphic_vector<T>
		{
			using value_type = T;
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
				(void)std::initializer_list<int>{ ((void)push_back_property<properties_ts>(value), 0)... };
			}
			template <typename property_t>
			void push_back_property(value_type * value)
			{
				content_sorted_accessor[gcl::type_info::deprecated::id<property_t>::value].push_back(value);
			}
		};
	}
}
