#pragma once

namespace gcl::container
{
	template <typename element_type, typename ... Ts>
	auto make_vector(Ts && ... args)
	{	// avoid vector elements copy-initialization
		using array_type = std::array<element_type, sizeof...(Ts)>;
		array_type values_as_array{ std::move(element_type{ std::forward<Ts>(args) })... };

		return std::vector<element_type>
		{
			std::move_iterator<array_type::iterator>(std::begin(values_as_array)),
			std::move_iterator<array_type::iterator>(std::end(values_as_array))
		};
	}
}