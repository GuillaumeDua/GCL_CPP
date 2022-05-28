#pragma once

#include <array>
#include <vector>
#include <variant>
#include <gcl_cpp/mp.hpp>

namespace gcl::container
{
	template <typename element_type, typename ... Ts>
	auto make_vector(Ts && ... args)
	{	// avoid vector elements copy-initialization
		using array_type = std::array<element_type, sizeof...(Ts)>;

		static_assert((not std::is_same_v<element_type, Ts> && ...));
		static_assert((std::is_constructible_v<element_type, Ts&&> && ...));
		static_assert(std::is_move_constructible_v<element_type>);

		array_type values_as_array{ std::move(element_type{ std::forward<Ts>(args) })... };

		// if brace-initializer :
		// static_assert(not std::is_constructible_v<element_type, std::move_iterator<array_type::iterator>>, "gcl::container::make_vector : ambiguous constructor may lead to unexpected value");

		return std::vector<element_type>
		(
			std::move_iterator<array_type::iterator>(std::begin(values_as_array)),
			std::move_iterator<array_type::iterator>(std::end(values_as_array))
		);
	}

	template <typename ... Ts>
	auto make_variant_array(Ts && ... values)
	{
		static_assert(gcl::mp::are_unique<Ts...>, "gcl::container::make_variant_array : duplicate in std::variant parameter-types");

		using container_element_type = std::variant<Ts...>;
		using container_type = std::array<container_element_type, sizeof...(Ts)>;

		return container_type{ std::forward<Ts>(values)... };
	}

	template <typename T>
	static auto to_ref(std::vector<T*> & container)
	{	// todo : dig that idea. costly as-is
		std::vector<std::reference_wrapper<T>> container_as_ref;
		for (auto & elem : container)
			container_as_ref.emplace_back(*elem);
		return container_as_ref;
	}

}